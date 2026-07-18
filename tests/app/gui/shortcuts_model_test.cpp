#include "gui/shortcuts_model.hpp"

#include <firelight/input/shortcut_registry.hpp>

#include <gtest/gtest.h>

namespace firelight::gui {

// The editor's preset behavior: which rows read as modified, resetting one row,
// and applying a different preset wholesale
class ShortcutsModelTest : public testing::Test {
protected:
  std::shared_ptr<input::ShortcutMapping> m_mapping;

  void SetUp() override {
    input::ShortcutRegistry::instance().loadFromString(R"({
      "defaultPreset": "firelight",
      "actions": [
        {"id": "fast_forward", "label": "Fast forward", "category": "Speed",
         "activation": "hold", "scope": "in-game"},
        {"id": "save_state", "label": "Save state", "category": "Save states",
         "activation": "press", "scope": "in-game"},
        {"id": "screenshot", "label": "Screenshot", "category": "System",
         "activation": "press", "scope": "always"}
      ],
      "presets": [
        {"id": "firelight", "label": "Firelight",
         "gamepad": {"fast_forward": ["Select+RightTrigger"],
                     "screenshot": ["Select+DpadUp"]},
         "keyboard": {"save_state": ["Key_F1"]}},
        {"id": "handheld", "label": "Handheld",
         "gamepad": {"fast_forward": ["Select+SouthFace"]}},
        {"id": "none", "label": "Empty", "gamepad": {}, "keyboard": {}}
      ]
    })");
    m_mapping = std::make_shared<input::ShortcutMapping>();
  }

  void TearDown() override { input::ShortcutRegistry::instance().clear(); }

  // profileId -1 keeps the model off the (absent) repository; the preset it
  // measures against is passed in directly
  std::unique_ptr<ShortcutsModel> makeModel(const bool isKeyboard,
                                            const std::string &presetId) {
    return std::make_unique<ShortcutsModel>(-1, isKeyboard, m_mapping,
                                            presetId);
  }

  static QVariant valueAt(const ShortcutsModel &model, const QString &id,
                          const char *role) {
    const auto roles = model.roleNames();
    int wanted = -1;
    for (auto it = roles.begin(); it != roles.end(); ++it) {
      if (it.value() == role) {
        wanted = it.key();
      }
    }
    for (int row = 0; row < model.rowCount({}); ++row) {
      const auto index = model.index(row);
      if (model.data(index, roles.key("shortcutId")).toString() == id) {
        return model.data(index, wanted);
      }
    }
    return {};
  }
};

TEST_F(ShortcutsModelTest, SeededRowsMatchThePresetAndReadUnmodified) {
  m_mapping->setBindings(
      "fast_forward",
      {*input::parseInputSource("Select+RightTrigger", DeviceType::Gamepad)});
  m_mapping->setBindings(
      "screenshot",
      {*input::parseInputSource("Select+DpadUp", DeviceType::Gamepad)});

  const auto model = makeModel(false, "firelight");
  EXPECT_FALSE(valueAt(*model, "fast_forward", "isModified").toBool());
  EXPECT_FALSE(valueAt(*model, "screenshot", "isModified").toBool());
  // The preset binds nothing here, and neither does the profile
  EXPECT_FALSE(valueAt(*model, "save_state", "isModified").toBool());
}

TEST_F(ShortcutsModelTest, RebindingMarksTheRowModified) {
  const auto model = makeModel(false, "firelight");
  model->applyPreset("firelight"); // a real profile is seeded at creation
  model->addBinding("fast_forward", {}, static_cast<int>(input::GamepadInput::SouthFace));

  EXPECT_TRUE(valueAt(*model, "fast_forward", "isModified").toBool());
  EXPECT_FALSE(valueAt(*model, "screenshot", "isModified").toBool());
}

// Clearing is how a shortcut is disabled, so it counts as modified against a
// preset that binds it — otherwise Reset would look like a no-op
TEST_F(ShortcutsModelTest, ClearingAPresetBoundRowMarksItModified) {
  m_mapping->setBindings(
      "fast_forward",
      {*input::parseInputSource("Select+RightTrigger", DeviceType::Gamepad)});

  const auto model = makeModel(false, "firelight");
  model->clearBindings("fast_forward");

  EXPECT_FALSE(valueAt(*model, "fast_forward", "hasBinding").toBool());
  EXPECT_TRUE(valueAt(*model, "fast_forward", "isModified").toBool());
}

TEST_F(ShortcutsModelTest, ResetToDefaultRestoresThePresetBinding) {
  const auto model = makeModel(false, "firelight");
  model->addBinding("fast_forward", {}, static_cast<int>(input::GamepadInput::SouthFace));
  ASSERT_TRUE(valueAt(*model, "fast_forward", "isModified").toBool());

  model->resetToDefault("fast_forward");

  EXPECT_FALSE(valueAt(*model, "fast_forward", "isModified").toBool());
  EXPECT_EQ(m_mapping->getBindings("fast_forward"),
            std::vector{*input::parseInputSource("Select+RightTrigger",
                                                 DeviceType::Gamepad)});
}

// Reset is a write, not an erase: for a row the preset leaves alone, it means
// unbound
TEST_F(ShortcutsModelTest, ResetOnAnUnseededRowUnbindsIt) {
  const auto model = makeModel(false, "firelight");
  model->addBinding("save_state", {}, static_cast<int>(input::GamepadInput::NorthFace));
  ASSERT_TRUE(valueAt(*model, "save_state", "hasBinding").toBool());

  model->resetToDefault("save_state");

  EXPECT_FALSE(valueAt(*model, "save_state", "hasBinding").toBool());
  EXPECT_FALSE(valueAt(*model, "save_state", "isModified").toBool());
}

TEST_F(ShortcutsModelTest, ApplyPresetRebindsAndAdoptsTheNewPreset) {
  const auto model = makeModel(false, "firelight");
  ASSERT_EQ("firelight", model->presetId().toStdString());

  model->applyPreset("handheld");

  EXPECT_EQ("handheld", model->presetId().toStdString());
  EXPECT_EQ(m_mapping->getBindings("fast_forward"),
            std::vector{*input::parseInputSource("Select+SouthFace", DeviceType::Gamepad)});
  // Everything reads unmodified against the preset just applied
  EXPECT_FALSE(valueAt(*model, "fast_forward", "isModified").toBool());
}

// A preset that doesn't name an action wants it unbound; the previous preset's
// binding must not survive
TEST_F(ShortcutsModelTest, ApplyPresetClearsRowsTheNewPresetOmits) {
  const auto model = makeModel(false, "firelight");
  model->applyPreset("firelight");
  ASSERT_TRUE(valueAt(*model, "screenshot", "hasBinding").toBool());

  model->applyPreset("handheld"); // binds fast_forward only

  EXPECT_FALSE(valueAt(*model, "screenshot", "hasBinding").toBool());
  EXPECT_TRUE(valueAt(*model, "fast_forward", "hasBinding").toBool());
}

TEST_F(ShortcutsModelTest, KeyboardProfileReadsTheKeyboardHalfOfThePreset) {
  const auto model = makeModel(true, "firelight");
  model->applyPreset("firelight");

  EXPECT_TRUE(valueAt(*model, "save_state", "hasBinding").toBool());
  // fast_forward is bound for the gamepad only
  EXPECT_FALSE(valueAt(*model, "fast_forward", "hasBinding").toBool());
}

TEST_F(ShortcutsModelTest, PresetOptionsAreFilteredByDeviceKind) {
  const auto pad = makeModel(false, "firelight");
  const auto keys = makeModel(true, "firelight");

  const auto idsOf = [](const QVariantList &options) {
    QStringList ids;
    for (const auto &option : options) {
      ids << option.toMap().value("id").toString();
    }
    return ids;
  };

  // "handheld" is gamepad-only, so a keyboard is never offered it
  EXPECT_TRUE(idsOf(pad->presetOptions()).contains("handheld"));
  EXPECT_FALSE(idsOf(keys->presetOptions()).contains("handheld"));
  EXPECT_TRUE(idsOf(keys->presetOptions()).contains("firelight"));
}

// An unknown preset must leave the bindings alone rather than wipe them
TEST_F(ShortcutsModelTest, ApplyingAnUnknownPresetDoesNothing) {
  const auto model = makeModel(false, "firelight");
  model->applyPreset("firelight");

  model->applyPreset("not-a-preset");

  EXPECT_EQ("firelight", model->presetId().toStdString());
  EXPECT_TRUE(valueAt(*model, "fast_forward", "hasBinding").toBool());
}

} // namespace firelight::gui

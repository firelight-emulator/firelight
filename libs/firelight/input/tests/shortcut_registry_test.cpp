#include <firelight/input/shortcut_registry.hpp>

#include <QtCore/qnamespace.h>
#include <gtest/gtest.h>

namespace firelight::input {

class ShortcutRegistryTest : public testing::Test {
protected:
  void SetUp() override { ShortcutRegistry::instance().clear(); }
  void TearDown() override { ShortcutRegistry::instance().clear(); }

  static ShortcutAction make(const char *id, const char *name,
                             const char *category) {
    ShortcutAction action;
    action.id = id;
    action.displayName = name;
    action.category = category;
    return action;
  }
};

TEST_F(ShortcutRegistryTest, RegisterAndGet) {
  auto &registry = ShortcutRegistry::instance();
  registry.registerAction(make("save_state", "Save state", "Save States"));

  const auto *action = registry.getAction("save_state");
  ASSERT_NE(action, nullptr);
  EXPECT_EQ(action->displayName, "Save state");
  EXPECT_EQ(registry.getAction("does_not_exist"), nullptr);
}

TEST_F(ShortcutRegistryTest, RegisterOverwritesById) {
  auto &registry = ShortcutRegistry::instance();
  registry.registerAction(make("ff", "Fast forward", "Speed"));
  registry.registerAction(make("ff", "Turbo", "Speed"));

  ASSERT_NE(registry.getAction("ff"), nullptr);
  EXPECT_EQ(registry.getAction("ff")->displayName, "Turbo");
  EXPECT_EQ(registry.listActions().size(), 1u);
}

TEST_F(ShortcutRegistryTest, ListAndClear) {
  auto &registry = ShortcutRegistry::instance();
  registry.registerAction(make("a", "A", "X"));
  registry.registerAction(make("b", "B", "X"));
  EXPECT_EQ(registry.listActions().size(), 2u);

  registry.clear();
  EXPECT_TRUE(registry.listActions().empty());
}

TEST_F(ShortcutRegistryTest, ParsesBareInputAndCombo) {
  const auto bare = parseInputSource("R3", DeviceType::Gamepad);
  ASSERT_TRUE(bare.has_value());
  EXPECT_EQ(bare->type, SourceType::Button);
  EXPECT_EQ(bare->code, GamepadInput::R3);
  EXPECT_TRUE(bare->modifiers.empty());

  // Everything before the last '+' is a modifier that must be held.
  const auto combo = parseInputSource("Select+RightTrigger", DeviceType::Gamepad);
  ASSERT_TRUE(combo.has_value());
  EXPECT_EQ(combo->code, GamepadInput::RightTrigger);
  ASSERT_EQ(combo->modifiers.size(), 1u);
  EXPECT_EQ(combo->modifiers[0], GamepadInput::Select);

  const auto key = parseInputSource("Key_F1", DeviceType::Keyboard);
  ASSERT_TRUE(key.has_value());
  EXPECT_EQ(key->type, SourceType::Key);
  EXPECT_EQ(key->code, Qt::Key_F1);
}

TEST_F(ShortcutRegistryTest, RejectsNamesTheDeviceDoesNotHave) {
  // A name is resolved against the device it's for, so neither vocabulary leaks
  // into the other.
  EXPECT_FALSE(parseInputSource("Key_F1", DeviceType::Gamepad).has_value());
  EXPECT_FALSE(parseInputSource("R3", DeviceType::Keyboard).has_value());
  EXPECT_FALSE(parseInputSource("Nonsense", DeviceType::Gamepad).has_value());
  EXPECT_FALSE(parseInputSource("", DeviceType::Gamepad).has_value());
  // A bad modifier fails the whole binding rather than silently dropping it.
  EXPECT_FALSE(parseInputSource("Nope+R3", DeviceType::Gamepad).has_value());
}

TEST_F(ShortcutRegistryTest, LoadsActionsAndPresets) {
  auto &registry = ShortcutRegistry::instance();
  ASSERT_TRUE(registry.loadFromString(R"({
    "defaultPreset": "mine",
    "actions": [
      {"id": "fast_forward", "label": "Fast forward", "category": "Speed",
       "activation": "hold", "scope": "in-game"},
      {"id": "screenshot", "label": "Screenshot", "category": "System",
       "activation": "press", "scope": "always"}
    ],
    "presets": [
      {"id": "mine", "label": "Mine",
       "gamepad": {"fast_forward": ["Select+RightTrigger", "Select+R3"]},
       "keyboard": {"screenshot": ["Key_F12"]}}
    ]
  })"));

  const auto *action = registry.getAction("fast_forward");
  ASSERT_NE(action, nullptr);
  EXPECT_EQ(action->activation, ActivationType::Hold);
  EXPECT_EQ(action->scope, ScopeInGame);
  EXPECT_EQ(registry.getAction("screenshot")->scope, ScopeAlways);

  EXPECT_EQ(registry.defaultPresetId(), "mine");
  const auto *preset = registry.findPreset("mine");
  ASSERT_NE(preset, nullptr);
  EXPECT_TRUE(preset->appliesTo(DeviceType::Gamepad));
  EXPECT_TRUE(preset->appliesTo(DeviceType::Keyboard));

  // Several sources per action, in preference order: seeding keeps the ones the
  // controller actually has.
  const auto &sources =
      preset->sourcesFor(DeviceType::Gamepad, "fast_forward");
  ASSERT_EQ(sources.size(), 2u);
  EXPECT_EQ(sources[0].code, GamepadInput::RightTrigger);
  EXPECT_EQ(sources[1].code, GamepadInput::R3);

  EXPECT_TRUE(preset->sourcesFor(DeviceType::Gamepad, "screenshot").empty());
  EXPECT_TRUE(registry.validate().empty());
}

TEST_F(ShortcutRegistryTest, RefusesAFileItCannotHonor) {
  auto &registry = ShortcutRegistry::instance();
  EXPECT_FALSE(registry.loadFromString("not json"));
  EXPECT_FALSE(registry.loadFromString(
      R"({"actions": [{"id": "a", "activation": "wat"}]})"));
  EXPECT_FALSE(
      registry.loadFromString(R"({"actions": [{"id": "a", "scope": "wat"}]})"));
  EXPECT_FALSE(registry.loadFromString(R"({"actions": [{"label": "no id"}]})"));
  // A binding naming an input the device hasn't got is a mistake in the file,
  // not something to quietly skip.
  EXPECT_FALSE(registry.loadFromString(R"({
    "actions": [{"id": "a"}],
    "presets": [{"id": "p", "gamepad": {"a": ["Key_F1"]}}]
  })"));
}

TEST_F(ShortcutRegistryTest, ValidateReportsWhatWouldSilentlyNotFire) {
  auto &registry = ShortcutRegistry::instance();

  // An action nobody declared: the engine skips unknown ids, so this would just
  // never fire, with nothing logged.
  ASSERT_TRUE(registry.loadFromString(R"({
    "defaultPreset": "p",
    "actions": [{"id": "real"}],
    "presets": [{"id": "p", "gamepad": {"typo": ["Select+R3"]}}]
  })"));
  auto problems = registry.validate();
  ASSERT_EQ(problems.size(), 1u);
  EXPECT_NE(problems[0].find("typo"), std::string::npos);

  // A keyboard shortcut sitting on a gameplay key would fire the shortcut and
  // press the button.
  ASSERT_TRUE(registry.loadFromString(R"({
    "defaultPreset": "p",
    "actions": [{"id": "real"}],
    "presets": [{"id": "p", "keyboard": {"real": ["Key_Z"]}}]
  })"));
  problems = registry.validate();
  ASSERT_EQ(problems.size(), 1u);
  EXPECT_NE(problems[0].find("gameplay"), std::string::npos);

  // A defaultPreset that doesn't exist means new profiles silently get nothing.
  ASSERT_TRUE(registry.loadFromString(R"({
    "defaultPreset": "missing",
    "actions": [{"id": "real"}],
    "presets": [{"id": "p"}]
  })"));
  problems = registry.validate();
  ASSERT_EQ(problems.size(), 1u);
  EXPECT_NE(problems[0].find("missing"), std::string::npos);
}

// The file we ship, not a fixture: it's the thing that decides whether hotkeys
// work on a fresh install.
TEST_F(ShortcutRegistryTest, ShippedCatalogIsGood) {
  auto &registry = ShortcutRegistry::instance();
  ASSERT_TRUE(registry.loadFromFile(FL_SHORTCUTS_FILE));

  const auto problems = registry.validate();
  for (const auto &problem : problems) {
    ADD_FAILURE() << "data/shortcuts.json: " << problem;
  }

  const auto *preset = registry.findPreset(registry.defaultPresetId());
  ASSERT_NE(preset, nullptr);
  EXPECT_TRUE(preset->appliesTo(DeviceType::Gamepad))
      << "the default preset has to bind something on a pad, or a fresh install "
         "has no working hotkeys";
  EXPECT_TRUE(preset->appliesTo(DeviceType::Keyboard));

  // Steam claims Guide globally and out of process, so binding it would look
  // broken through no fault of ours.
  for (const auto &[device, byId] : preset->bindings) {
    for (const auto &[id, sources] : byId) {
      for (const auto &source : sources) {
        EXPECT_NE(source.code, GamepadInput::Home)
            << id << " binds Guide/Home, which Steam takes over";
      }
    }
  }
}

} // namespace firelight::input

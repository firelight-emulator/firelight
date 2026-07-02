#include "gui/models/emulation_settings_model.hpp"

#include <firelight/settings/settings_catalog.hpp>
#include <firelight/settings/settings_service.hpp>
#include <firelight/settings/sqlite_settings_repository.hpp>

#include <gtest/gtest.h>

namespace firelight::settings {
namespace {

// Platform 3 = Game Boy Advance -> core "mgba_libretro".
constexpr int kGbaPlatformId = 3;

const char *kCatalog = R"JSON(
{
  "common": [
    {"key": "rewind-enabled", "label": "Rewind", "category": "Emulation",
     "type": "boolean", "default": "true"},
    {"key": "aspect-ratio", "label": "Aspect ratio", "category": "Video",
     "type": "options", "default": "corrected",
     "options": [{"label": "Pixel", "value": "pixel"},
                 {"label": "Corrected", "value": "corrected"}]}
  ],
  "cores": {
    "mgba_libretro": {
      "settings": [
        {"key": "solar-sensor", "label": "Solar sensor", "category": "Hardware",
         "type": "boolean", "default": "false"},
        {"key": "solar-level", "label": "Solar level", "category": "Hardware",
         "type": "slider", "default": "0", "min": 0, "max": 10, "step": 1,
         "visibleWhen": [{"key": "solar-sensor", "values": ["true"]}]},
        {"key": "adv-opt", "label": "Advanced option", "category": "Hardware",
         "type": "boolean", "default": "false", "advanced": true}
      ]
    }
  }
}
)JSON";

int roleFor(const EmulationSettingsModel &model, const QByteArray &name) {
  const auto roles = model.roleNames();
  for (auto it = roles.cbegin(); it != roles.cend(); ++it) {
    if (it.value() == name) {
      return it.key();
    }
  }
  return -1;
}

int findRow(const EmulationSettingsModel &model, const QString &key) {
  const int keyRole = roleFor(model, "key");
  for (int i = 0; i < model.rowCount({}); ++i) {
    if (model.data(model.index(i), keyRole).toString() == key) {
      return i;
    }
  }
  return -1;
}
} // namespace

class EmulationSettingsModelTest : public testing::Test {
protected:
  SqliteSettingsRepository m_repo{":memory:"};
  SettingsService m_service{m_repo};

  void SetUp() override {
    SettingsService::setInstance(&m_service);
    ASSERT_TRUE(SettingsCatalog::instance().loadFromJson(kCatalog));
  }

  void TearDown() override {
    SettingsCatalog::instance().loadFromJson("{}");
    SettingsService::setInstance(nullptr);
  }

  QVariant value(const EmulationSettingsModel &model, int row,
                 const QByteArray &role) {
    return model.data(model.index(row), roleFor(model, role));
  }
};

TEST_F(EmulationSettingsModelTest, GlobalShowsCommonSettingsOnly) {
  EmulationSettingsModel model;
  model.setLevel(Global);
  // No platform -> only the common settings.
  EXPECT_EQ(model.rowCount({}), 2);
}

TEST_F(EmulationSettingsModelTest, PlatformShowsCommonPlusCoreSettings) {
  EmulationSettingsModel model;
  model.setPlatformId(kGbaPlatformId);
  model.setLevel(Platform);
  EXPECT_EQ(model.rowCount({}), 5); // 2 common + 3 core (incl. advanced)
  EXPECT_NE(findRow(model, "solar-sensor"), -1);
}

TEST_F(EmulationSettingsModelTest, ExposesWidgetAndSliderBounds) {
  EmulationSettingsModel model;
  model.setPlatformId(kGbaPlatformId);
  model.setLevel(Platform);

  const int rewind = findRow(model, "rewind-enabled");
  ASSERT_NE(rewind, -1);
  EXPECT_EQ(value(model, rewind, "widget").toString(), "toggle");
  EXPECT_EQ(value(model, rewind, "value").toBool(), true); // default true

  const int level = findRow(model, "solar-level");
  ASSERT_NE(level, -1);
  EXPECT_EQ(value(model, level, "widget").toString(), "slider");
  EXPECT_DOUBLE_EQ(value(model, level, "maximumValue").toDouble(), 10.0);
  EXPECT_DOUBLE_EQ(value(model, level, "stepValue").toDouble(), 1.0);
}

TEST_F(EmulationSettingsModelTest, ShowsInheritedValueThenGameOverride) {
  m_service.setPlatformValue(kGbaPlatformId, "aspect-ratio", "pixel");

  EmulationSettingsModel model;
  model.setPlatformId(kGbaPlatformId);
  model.setContentHash("hash1");
  model.setLevel(Game);

  const int row = findRow(model, "aspect-ratio");
  ASSERT_NE(row, -1);
  // Inherited from platform; not overridden at the game tier.
  EXPECT_EQ(value(model, row, "value").toString(), "pixel");
  EXPECT_FALSE(value(model, row, "overridden").toBool());

  const int valueRole = roleFor(model, "value");
  ASSERT_TRUE(model.setData(model.index(row), "corrected", valueRole));
  EXPECT_EQ(value(model, row, "value").toString(), "corrected");
  EXPECT_TRUE(value(model, row, "overridden").toBool());
  EXPECT_EQ(m_service.getValueAtLevel(Game, "hash1", kGbaPlatformId,
                                      "aspect-ratio")
                .value_or(""),
            "corrected");
}

TEST_F(EmulationSettingsModelTest, ResetFallsBackToInherited) {
  m_service.setPlatformValue(kGbaPlatformId, "aspect-ratio", "pixel");

  EmulationSettingsModel model;
  model.setPlatformId(kGbaPlatformId);
  model.setContentHash("hash1");
  model.setLevel(Game);

  const int row = findRow(model, "aspect-ratio");
  const int valueRole = roleFor(model, "value");
  ASSERT_TRUE(model.setData(model.index(row), "corrected", valueRole));
  ASSERT_TRUE(value(model, row, "overridden").toBool());

  model.resetValue(row);
  EXPECT_EQ(value(model, row, "value").toString(), "pixel");
  EXPECT_FALSE(value(model, row, "overridden").toBool());
}

TEST_F(EmulationSettingsModelTest, VisibleWhenTracksDependency) {
  EmulationSettingsModel model;
  model.setPlatformId(kGbaPlatformId);
  model.setContentHash("hash1");
  model.setLevel(Game);

  const int sensor = findRow(model, "solar-sensor");
  const int level = findRow(model, "solar-level");
  ASSERT_NE(sensor, -1);
  ASSERT_NE(level, -1);

  // solar-sensor defaults false -> solar-level hidden.
  EXPECT_FALSE(value(model, level, "visible").toBool());

  // Turn the sensor on -> the dependent slider becomes visible.
  const int valueRole = roleFor(model, "value");
  ASSERT_TRUE(model.setData(model.index(sensor), true, valueRole));
  EXPECT_TRUE(value(model, level, "visible").toBool());
}

TEST_F(EmulationSettingsModelTest, AdvancedHiddenUnlessShowAdvanced) {
  EmulationSettingsModel model;
  model.setPlatformId(kGbaPlatformId);
  model.setLevel(Platform);

  const int adv = findRow(model, "adv-opt");
  ASSERT_NE(adv, -1);

  // Advanced settings are hidden by default (showAdvanced defaults false).
  EXPECT_FALSE(value(model, adv, "visible").toBool());

  model.setShowAdvanced(true);
  EXPECT_TRUE(value(model, adv, "visible").toBool());

  model.setShowAdvanced(false);
  EXPECT_FALSE(value(model, adv, "visible").toBool());
}

} // namespace firelight::settings

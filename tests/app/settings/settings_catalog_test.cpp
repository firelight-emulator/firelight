#include <firelight/settings/settings_catalog.hpp>

#include <gtest/gtest.h>

#include <algorithm>

namespace firelight::settings {

namespace {
const char *kSampleJson = R"JSON(
{
  "common": [
    {"key": "rewind-enabled", "label": "Rewind", "type": "boolean",
     "default": "true"},
    {"key": "aspect-ratio", "label": "Aspect ratio", "type": "options",
     "default": "emulator-corrected",
     "options": [
       {"label": "Pixel perfect", "value": "pixel-perfect"},
       {"label": "Corrected", "value": "emulator-corrected"}
     ]}
  ],
  "cores": {
    "mgba_libretro": {
      "settings": [
        {"key": "solar-sensor", "label": "Solar sensor", "type": "boolean",
         "default": "false",
         "mapping": [
           {"coreKey": "mgba_solar_sensor_level",
            "values": {"true": "5", "false": "0"}}
         ]},
        {"key": "solar-level", "label": "Solar level", "type": "options",
         "default": "0",
         "options": [{"label": "0", "value": "0"}, {"label": "5", "value": "5"}],
         "visibleWhen": [{"key": "solar-sensor", "values": ["true"]}]}
      ],
      "defaults": {
        "mgba_color_correction": "OFF",
        "mgba_frameskip": "disabled"
      }
    }
  }
}
)JSON";

const EmulationSetting *find(const std::vector<EmulationSetting> &v,
                             const std::string &key) {
  const auto it = std::find_if(v.begin(), v.end(),
                               [&](const auto &s) { return s.key == key; });
  return it != v.end() ? &*it : nullptr;
}
} // namespace

class SettingsCatalogTest : public testing::Test {
protected:
  SettingsCatalog catalog;
  void SetUp() override { ASSERT_TRUE(catalog.loadFromJson(kSampleJson)); }
};

TEST_F(SettingsCatalogTest, ParsesCommonAndPerCore) {
  EXPECT_EQ(catalog.commonSettings().size(), 2u);
  EXPECT_EQ(catalog.coreSpecificSettings("mgba_libretro").size(), 2u);
  EXPECT_TRUE(catalog.coreSpecificSettings("unknown_core").empty());
  // common followed by the core-specific settings.
  EXPECT_EQ(catalog.settingsForCore("mgba_libretro").size(), 4u);
}

TEST_F(SettingsCatalogTest, ParsesCoreDefaults) {
  const auto &defaults = catalog.coreDefaults("mgba_libretro");
  EXPECT_EQ(defaults.size(), 2u);
  ASSERT_TRUE(defaults.count("mgba_color_correction"));
  EXPECT_EQ(defaults.at("mgba_color_correction"), "OFF");
  EXPECT_TRUE(catalog.coreDefaults("unknown_core").empty());
}

TEST_F(SettingsCatalogTest, ParsesOptionsAndBoolean) {
  const auto &common = catalog.commonSettings();
  const auto *rewind = find(common, "rewind-enabled");
  ASSERT_NE(rewind, nullptr);
  EXPECT_EQ(rewind->type, BOOLEAN);

  const auto *aspect = find(common, "aspect-ratio");
  ASSERT_NE(aspect, nullptr);
  EXPECT_EQ(aspect->type, OPTIONS);
  ASSERT_EQ(aspect->options.size(), 2u);
  EXPECT_EQ(aspect->options[0].value, "pixel-perfect");
}

TEST_F(SettingsCatalogTest, ParsesMappingWithValueTranslation) {
  const auto *solar = find(catalog.coreSpecificSettings("mgba_libretro"), "solar-sensor");
  ASSERT_NE(solar, nullptr);
  const auto mapped = resolveCoreOptionValues(*solar, "true");
  ASSERT_EQ(mapped.size(), 1u);
  EXPECT_EQ(mapped[0].first, "mgba_solar_sensor_level");
  EXPECT_EQ(mapped[0].second, "5");
}

TEST_F(SettingsCatalogTest, ParsesVisibilityDependency) {
  const auto *solarLevel =
      find(catalog.coreSpecificSettings("mgba_libretro"), "solar-level");
  ASSERT_NE(solarLevel, nullptr);
  ASSERT_EQ(solarLevel->visibleWhen.size(), 1u);

  const SettingValueResolver off = [](const std::string &) { return "false"; };
  const SettingValueResolver on = [](const std::string &) { return "true"; };
  EXPECT_FALSE(settingIsVisible(*solarLevel, off));
  EXPECT_TRUE(settingIsVisible(*solarLevel, on));
}

TEST(SettingsCatalogSyncTest, ParsesSyncMethodAndTargetFramerate) {
  SettingsCatalog catalog;
  ASSERT_TRUE(catalog.loadFromJson(R"JSON(
{
  "common": [
    { "key": "sync-method", "type": "options", "default": "audio",
      "options": [
        { "label": "Audio", "value": "audio" },
        { "label": "Sync to monitor", "value": "monitor" },
        { "label": "Native timer", "value": "native" },
        { "label": "Fixed framerate", "value": "fixed" }
      ] },
    { "key": "target-framerate", "type": "spinbox", "default": "60",
      "min": 30, "max": 240, "step": 1, "advanced": true,
      "enabledWhen": [ { "key": "sync-method", "values": ["fixed"] } ] }
  ],
  "cores": {}
}
)JSON"));

  const auto &common = catalog.commonSettings();

  const auto *sync = find(common, "sync-method");
  ASSERT_NE(sync, nullptr);
  EXPECT_EQ(sync->type, OPTIONS);
  EXPECT_EQ(sync->defaultValue, "audio");
  ASSERT_EQ(sync->options.size(), 4u);
  EXPECT_FALSE(sync->advanced); // defaults to false when omitted

  const auto *fps = find(common, "target-framerate");
  ASSERT_NE(fps, nullptr);
  EXPECT_EQ(fps->type, INTEGER);
  EXPECT_EQ(fps->widget, "spinbox");
  EXPECT_TRUE(fps->advanced); // "advanced": true parsed
  EXPECT_DOUBLE_EQ(fps->minValue, 30.0);
  EXPECT_DOUBLE_EQ(fps->maxValue, 240.0);
  // Only editable when sync-method is "fixed".
  ASSERT_EQ(fps->enabledWhen.size(), 1u);
  EXPECT_EQ(fps->enabledWhen[0].key, "sync-method");
  ASSERT_EQ(fps->enabledWhen[0].values.size(), 1u);
  EXPECT_EQ(fps->enabledWhen[0].values[0], "fixed");

  const SettingValueResolver fixed = [](const std::string &) { return "fixed"; };
  const SettingValueResolver audio = [](const std::string &) { return "audio"; };
  EXPECT_TRUE(settingIsEnabled(*fps, fixed));
  EXPECT_FALSE(settingIsEnabled(*fps, audio));
}

TEST_F(SettingsCatalogTest, InvalidJsonLeavesCatalogIntact) {
  EXPECT_FALSE(catalog.loadFromJson("{ this is not valid"));
  EXPECT_EQ(catalog.commonSettings().size(), 2u);
}

TEST(SettingsCatalogTypesTest, ParsesSliderAndCustomWidgets) {
  SettingsCatalog c;
  ASSERT_TRUE(c.loadFromJson(R"JSON(
  {
    "cores": {
      "mgba_libretro": { "settings": [
        {"key": "level", "label": "Level", "type": "slider", "default": "0",
         "min": 0, "max": 10, "step": 2},
        {"key": "count", "label": "Count", "type": "number", "default": "1"},
        {"key": "palette", "label": "Palette", "type": "custom",
         "widget": "gbc-palette", "default": "a",
         "options": [{"label": "A", "value": "a"}]},
        {"key": "volume", "label": "Volume", "type": "slider", "default": "1.0",
         "min": 0.0, "max": 2.0, "step": 0.25},
        {"key": "big-step", "label": "Big step", "type": "slider", "default": "0",
         "min": 0, "max": 100, "step": 5},
        {"key": "plain", "label": "Plain", "type": "slider", "default": "0",
         "min": 0, "max": 10}
      ]}
    }
  }
  )JSON"));

  const auto &settings = c.coreSpecificSettings("mgba_libretro");
  ASSERT_EQ(settings.size(), 6u);

  EXPECT_EQ(settings[0].type, INTEGER);
  EXPECT_EQ(settings[0].widget, "slider");
  EXPECT_DOUBLE_EQ(settings[0].minValue, 0.0);
  EXPECT_DOUBLE_EQ(settings[0].maxValue, 10.0);
  EXPECT_DOUBLE_EQ(settings[0].stepValue, 2.0);

  // "number" is INTEGER value semantics with a spinbox widget.
  EXPECT_EQ(settings[1].type, INTEGER);
  EXPECT_EQ(settings[1].widget, "spinbox");

  EXPECT_EQ(settings[2].type, CUSTOM);
  EXPECT_EQ(settings[2].widget, "gbc-palette");

  // Fractional step -> float-valued slider.
  EXPECT_DOUBLE_EQ(settings[3].minValue, 0.0);
  EXPECT_DOUBLE_EQ(settings[3].maxValue, 2.0);
  EXPECT_DOUBLE_EQ(settings[3].stepValue, 0.25);

  // Integer step other than 1 (e.g. increments of 5).
  EXPECT_DOUBLE_EQ(settings[4].stepValue, 5.0);

  // Omitted step defaults to 1.
  EXPECT_DOUBLE_EQ(settings[5].stepValue, 1.0);
}

} // namespace firelight::settings

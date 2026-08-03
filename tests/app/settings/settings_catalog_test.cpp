#include "audio/audio_manager.hpp"

#include <firelight/settings/settings_catalog.hpp>

#include <algorithm>
#include <gtest/gtest.h>

namespace firelight::settings {

namespace {
const char *SAMPLE_JSON = R"JSON(
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

const SettingDefinition *find(const std::vector<SettingDefinition> &v, const std::string &key) {
  const auto it = std::find_if(v.begin(), v.end(), [&](const auto &s) { return s.key == key; });
  return it != v.end() ? &*it : nullptr;
}
} // namespace

class SettingsCatalogTest : public testing::Test {
protected:
  SettingsCatalog catalog;

  void SetUp() override { ASSERT_TRUE(catalog.loadFromJson(SAMPLE_JSON)); }
};

TEST_F(SettingsCatalogTest, ParsesCommonAndPerCore) {
  EXPECT_EQ(catalog.commonSettings().size(), 2u);
  EXPECT_EQ(catalog.coreSpecificSettings("mgba_libretro").size(), 2u);
  EXPECT_TRUE(catalog.coreSpecificSettings("unknown_core").empty());
  // common followed by the core-specific settings
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
  EXPECT_EQ(rewind->type, SettingType::BOOLEAN);

  const auto *aspect = find(common, "aspect-ratio");
  ASSERT_NE(aspect, nullptr);
  EXPECT_EQ(aspect->type, SettingType::OPTIONS);
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
  const auto *solarLevel = find(catalog.coreSpecificSettings("mgba_libretro"), "solar-level");
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
  EXPECT_EQ(sync->type, SettingType::OPTIONS);
  EXPECT_EQ(sync->defaultValue, "audio");
  ASSERT_EQ(sync->options.size(), 4u);
  EXPECT_FALSE(sync->advanced); // defaults to false when omitted

  const auto *fps = find(common, "target-framerate");
  ASSERT_NE(fps, nullptr);
  EXPECT_EQ(fps->type, SettingType::INTEGER);
  EXPECT_EQ(fps->widget, "spinbox");
  EXPECT_TRUE(fps->advanced); // "advanced": true parsed
  EXPECT_DOUBLE_EQ(fps->minValue, 30.0);
  EXPECT_DOUBLE_EQ(fps->maxValue, 240.0);
  // Only editable when sync-method is "fixed"
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

  EXPECT_EQ(settings[0].type, SettingType::INTEGER);
  EXPECT_EQ(settings[0].widget, "slider");
  EXPECT_DOUBLE_EQ(settings[0].minValue, 0.0);
  EXPECT_DOUBLE_EQ(settings[0].maxValue, 10.0);
  EXPECT_DOUBLE_EQ(settings[0].stepValue, 2.0);

  // "number" is INTEGER value semantics with a spinbox widget
  EXPECT_EQ(settings[1].type, SettingType::INTEGER);
  EXPECT_EQ(settings[1].widget, "spinbox");

  EXPECT_EQ(settings[2].type, SettingType::CUSTOM);
  EXPECT_EQ(settings[2].widget, "gbc-palette");

  // Fractional step -> float-valued slider
  EXPECT_DOUBLE_EQ(settings[3].minValue, 0.0);
  EXPECT_DOUBLE_EQ(settings[3].maxValue, 2.0);
  EXPECT_DOUBLE_EQ(settings[3].stepValue, 0.25);

  // Integer step other than 1 (e.g. increments of 5)
  EXPECT_DOUBLE_EQ(settings[4].stepValue, 5.0);

  // Omitted step defaults to 1
  EXPECT_DOUBLE_EQ(settings[5].stepValue, 1.0);
}

TEST(SettingsCatalogGamePickerTest, ParsesLibraryGamePicker) {
  SettingsCatalog c;
  ASSERT_TRUE(c.loadFromJson(R"JSON(
  {
    "cores": {
      "mupen64plus_next_libretro": { "settings": [
        {"key": "transfer-pak-game", "label": "Transfer Pak cartridge",
         "type": "game-picker", "eligiblePlatformIds": [1, 2], "default": ""}
      ]}
    }
  }
  )JSON"));

  const auto *picker = find(c.coreSpecificSettings("mupen64plus_next_libretro"), "transfer-pak-game");
  ASSERT_NE(picker, nullptr);
  // Value semantics are OPTIONS, rendered by the dropdown widget
  EXPECT_EQ(picker->type, SettingType::OPTIONS);
  EXPECT_EQ(picker->widget, "dropdown");
  EXPECT_TRUE(picker->libraryGameSource);
  // Options are NOT authored in the catalog (the app fills them at runtime)
  EXPECT_TRUE(picker->options.empty());
  ASSERT_EQ(picker->gamePickerPlatformIds.size(), 2u);
  EXPECT_EQ(picker->gamePickerPlatformIds[0], 1);
  EXPECT_EQ(picker->gamePickerPlatformIds[1], 2);
}

TEST(SettingsCatalogWidgetTypesTest, ParsesTextColorFileFolderMultiSelect) {
  SettingsCatalog c;
  ASSERT_TRUE(c.loadFromJson(R"JSON(
  {
    "cores": {
      "mgba_libretro": { "settings": [
        {"key": "title", "label": "Title", "type": "text",
         "placeholder": "Enter a name", "default": ""},
        {"key": "tint", "label": "Tint", "type": "color", "default": "#ff8800"},
        {"key": "bios", "label": "BIOS", "type": "file-picker",
         "extensions": ["bin", "bios"], "default": ""},
        {"key": "romdir", "label": "ROM folder", "type": "folder-picker",
         "default": ""},
        {"key": "cheats", "label": "Cheats", "type": "multi-select",
         "default": "[]",
         "options": [{"label": "A", "value": "a"}, {"label": "B", "value": "b"}]}
      ]}
    }
  }
  )JSON"));

  const auto &s = c.coreSpecificSettings("mgba_libretro");

  const auto *text = find(s, "title");
  ASSERT_NE(text, nullptr);
  EXPECT_EQ(text->type, SettingType::STRING);
  EXPECT_EQ(text->widget, "text");
  EXPECT_EQ(text->placeholder, "Enter a name");

  const auto *color = find(s, "tint");
  ASSERT_NE(color, nullptr);
  EXPECT_EQ(color->type, SettingType::STRING);
  EXPECT_EQ(color->widget, "color");

  const auto *file = find(s, "bios");
  ASSERT_NE(file, nullptr);
  EXPECT_EQ(file->type, SettingType::STRING);
  EXPECT_EQ(file->widget, "file-picker");
  EXPECT_FALSE(file->directoryMode);
  ASSERT_EQ(file->fileExtensions.size(), 2u);
  EXPECT_EQ(file->fileExtensions[0], "bin");

  const auto *folder = find(s, "romdir");
  ASSERT_NE(folder, nullptr);
  EXPECT_EQ(folder->widget, "folder-picker");
  EXPECT_TRUE(folder->directoryMode);

  const auto *multi = find(s, "cheats");
  ASSERT_NE(multi, nullptr);
  EXPECT_EQ(multi->type, SettingType::OPTIONS);
  EXPECT_EQ(multi->widget, "multi-select");
  ASSERT_EQ(multi->options.size(), 2u);
}

namespace {
const char *LAYOUT_JSON = R"JSON(
{
  "pages": [
    {"id": "emulation", "label": "Emulation", "icon": "chip",
     "route": "/settings/emulation", "order": 20, "keywords": ["core"]},
    {"id": "appearance", "label": "Appearance", "icon": "palette",
     "route": "/settings/appearance", "order": 10}
  ],
  "groups": [
    {"id": "video", "page": "emulation", "label": "Video", "order": 20},
    {"id": "theme", "page": "appearance", "label": "Theme", "order": 10}
  ],
  "app": [
    {"key": "accent-color", "group": "theme", "label": "Accent color",
     "type": "color", "default": "#f76b15", "keywords": ["highlight"],
     "order": 20},
    {"key": "background-mode", "group": "theme", "label": "Background",
     "type": "options", "default": "solid", "order": 10,
     "options": [{"label": "Solid", "value": "solid"}]}
  ],
  "common": [
    {"key": "aspect-ratio", "group": "video", "label": "Aspect ratio",
     "type": "options", "default": "corrected",
     "options": [{"label": "Corrected", "value": "corrected"}]}
  ],
  "cores": {
    "mgba_libretro": {
      "settings": [
        {"key": "gba-blend", "group": "video", "label": "Interframe blending",
         "type": "boolean", "default": "false"}
      ]
    }
  }
}
)JSON";
} // namespace

TEST(SettingsCatalogLayoutTest, ParsesPagesAndGroupsInOrder) {
  SettingsCatalog c;
  ASSERT_TRUE(c.loadFromJson(LAYOUT_JSON));

  // Both are sorted by `order`, not declaration order
  ASSERT_EQ(c.pages().size(), 2u);
  EXPECT_EQ(c.pages()[0].id, "appearance");
  EXPECT_EQ(c.pages()[1].id, "emulation");

  const auto *page = c.findPage("appearance");
  ASSERT_NE(page, nullptr);
  EXPECT_EQ(page->label, "Appearance");
  EXPECT_EQ(page->icon, "palette");
  EXPECT_EQ(page->route, "/settings/appearance");

  const auto *group = c.findGroup("theme");
  ASSERT_NE(group, nullptr);
  EXPECT_EQ(group->pageId, "appearance");
  EXPECT_EQ(group->label, "Theme");

  EXPECT_EQ(c.findPage("nope"), nullptr);
  EXPECT_EQ(c.findGroup("nope"), nullptr);
}

TEST(SettingsCatalogLayoutTest, ParsesAppSettingsWithKeywords) {
  SettingsCatalog c;
  ASSERT_TRUE(c.loadFromJson(LAYOUT_JSON));

  ASSERT_EQ(c.appSettings().size(), 2u);
  const auto *accent = find(c.appSettings(), "accent-color");
  ASSERT_NE(accent, nullptr);
  EXPECT_EQ(accent->groupId, "theme");
  EXPECT_EQ(accent->widget, "color");
  ASSERT_EQ(accent->keywords.size(), 1u);
  EXPECT_EQ(accent->keywords[0], "highlight");

  EXPECT_TRUE(c.isAppSetting("accent-color"));
  EXPECT_FALSE(c.isAppSetting("aspect-ratio"));
  EXPECT_FALSE(c.isAppSetting("nope"));
}

TEST(SettingsCatalogLayoutTest, GroupCollectsFromEveryArrayInOrder) {
  SettingsCatalog c;
  ASSERT_TRUE(c.loadFromJson(LAYOUT_JSON));

  // Sorted by `order`, so the later-declared background-mode comes first
  const auto theme = c.settingsForGroup("theme");
  ASSERT_EQ(theme.size(), 2u);
  EXPECT_EQ(theme[0].key, "background-mode");
  EXPECT_EQ(theme[1].key, "accent-color");

  // A group spanning `common` and the named core's settings gathers both
  const auto video = c.settingsForGroup("video", "mgba_libretro");
  ASSERT_EQ(video.size(), 2u);
  EXPECT_NE(find(video, "aspect-ratio"), nullptr);
  EXPECT_NE(find(video, "gba-blend"), nullptr);

  // No core in scope (the global tier) means the frontend settings only — not
  // every core's settings piled together
  const auto globalVideo = c.settingsForGroup("video");
  ASSERT_EQ(globalVideo.size(), 1u);
  EXPECT_EQ(globalVideo[0].key, "aspect-ratio");

  // A core that declares nothing in the group contributes nothing
  EXPECT_EQ(c.settingsForGroup("video", "gambatte_libretro").size(), 1u);

  EXPECT_TRUE(c.settingsForGroup("nope").empty());
  EXPECT_TRUE(c.settingsForGroup("").empty());
}

TEST(SettingsCatalogLayoutTest, FindsAndEnumeratesEverySetting) {
  SettingsCatalog c;
  ASSERT_TRUE(c.loadFromJson(LAYOUT_JSON));

  // app + common + every core's settings — what the search index is built from
  EXPECT_EQ(c.allSettings().size(), 4u);

  const auto *fromCore = c.findByKey("gba-blend");
  ASSERT_NE(fromCore, nullptr);
  EXPECT_EQ(fromCore->label, "Interframe blending");
  ASSERT_NE(c.findByKey("accent-color"), nullptr);
  EXPECT_EQ(c.findByKey("nope"), nullptr);
}

TEST(SettingsCatalogLayoutTest, AppSettingsDropCoreOnlyFields) {
  SettingsCatalog c;
  ASSERT_TRUE(c.loadFromJson(R"JSON(
  {
    "groups": [{"id": "theme", "label": "Theme"}],
    "app": [
      {"key": "fullscreen", "group": "theme", "label": "Fullscreen",
       "type": "boolean", "default": "false",
       "trueValue": "on", "falseValue": "off",
       "mapping": [{"coreKey": "some_core_key"}]}
    ]
  }
  )JSON"));

  // An app setting never reaches a core, so these are stripped rather than
  // left looking load-bearing
  const auto *s = find(c.appSettings(), "fullscreen");
  ASSERT_NE(s, nullptr);
  EXPECT_TRUE(s->mapping.empty());
  EXPECT_EQ(s->trueStringValue, "true");
  EXPECT_EQ(s->falseStringValue, "false");
}

TEST(SettingsCatalogValidationTest, CleanCatalogHasNoProblems) {
  SettingsCatalog c;
  ASSERT_TRUE(c.loadFromJson(LAYOUT_JSON));
  EXPECT_TRUE(c.validate().empty());
}

TEST(SettingsCatalogValidationTest, ReportsAuthoringMistakes) {
  const auto problemsFor = [](const char *json) {
    SettingsCatalog c;
    EXPECT_TRUE(c.loadFromJson(json));
    return c.validate();
  };

  // A key declared twice, across different arrays
  EXPECT_FALSE(problemsFor(R"JSON(
  {
    "app": [{"key": "dupe", "label": "A", "type": "boolean"}],
    "common": [{"key": "dupe", "label": "B", "type": "boolean"}]
  })JSON")
                   .empty());

  // A setting pointing at a group nobody declared
  EXPECT_FALSE(problemsFor(R"JSON(
  {
    "app": [{"key": "a", "group": "ghost", "label": "A", "type": "boolean"}]
  })JSON")
                   .empty());

  // A group pointing at a page nobody declared
  EXPECT_FALSE(problemsFor(R"JSON(
  {
    "groups": [{"id": "g", "page": "ghost", "label": "G"}]
  })JSON")
                   .empty());

  // custom with no widget to render it
  EXPECT_FALSE(problemsFor(R"JSON(
  {
    "common": [{"key": "a", "label": "A", "type": "custom"}]
  })JSON")
                   .empty());

  // An unknown type silently became a dropdown before validate() existed
  EXPECT_FALSE(problemsFor(R"JSON(
  {
    "common": [{"key": "a", "label": "A", "type": "sldier"}]
  })JSON")
                   .empty());

  // Duplicate page / group ids
  EXPECT_FALSE(problemsFor(R"JSON(
  {
    "pages": [{"id": "p", "label": "P"}, {"id": "p", "label": "P2"}]
  })JSON")
                   .empty());

  // A dropdown with nothing to pick
  EXPECT_FALSE(problemsFor(R"JSON(
  {
    "common": [{"key": "a", "label": "A", "type": "options"}]
  })JSON")
                   .empty());

  // A game picker sources its options at runtime, so it needs none authored
  EXPECT_TRUE(problemsFor(R"JSON(
  {
    "common": [{"key": "a", "label": "A", "type": "game-picker"}]
  })JSON")
                  .empty());
}

TEST(SettingsCatalogValidationTest, ParsesTypeAliasesForEveryDelegate) {
  SettingsCatalog c;
  ASSERT_TRUE(c.loadFromJson(R"JSON(
  {
    "common": [
      {"key": "seg", "label": "Seg", "type": "segmented",
       "options": [{"label": "A", "value": "a"}]},
      {"key": "rad", "label": "Rad", "type": "radio",
       "options": [{"label": "A", "value": "a"}]},
      {"key": "step", "label": "Step", "type": "stepper"},
      {"key": "bind", "label": "Bind", "type": "key-binding"}
    ]
  }
  )JSON"));
  EXPECT_TRUE(c.validate().empty());

  const auto &s = c.commonSettings();
  EXPECT_EQ(find(s, "seg")->widget, "segmented");
  EXPECT_EQ(find(s, "rad")->widget, "radio");
  EXPECT_EQ(find(s, "step")->widget, "stepper");
  EXPECT_EQ(find(s, "step")->type, SettingType::INTEGER);
  EXPECT_EQ(find(s, "bind")->widget, "key-binding");
}

// The catalog we actually ship. Every other test here feeds the parser inline
// JSON, so nothing caught a typo in the real file — and page/group ids are
// load-bearing, so a typo silently orphans a setting rather than erroring
TEST(ShippedSettingsCatalogTest, ParsesAndValidatesCleanly) {
  SettingsCatalog c;
  ASSERT_TRUE(c.loadFromFile(FL_SETTINGS_CATALOG_FILE)) << "could not read " << FL_SETTINGS_CATALOG_FILE;

  const auto problems = c.validate();
  for (const auto &problem : problems) {
    ADD_FAILURE() << "settings_catalog.json: " << problem;
  }

  EXPECT_FALSE(c.pages().empty());
  EXPECT_FALSE(c.groups().empty());
  EXPECT_FALSE(c.commonSettings().empty());

  // Every group lands on a page, and every setting lands in a group — that's
  // what lets a page auto-render and search say where a result lives
  for (const auto &group : c.groups()) {
    EXPECT_FALSE(group.pageId.empty()) << "group '" << group.id << "' names no page";
    EXPECT_FALSE(group.label.empty()) << "group '" << group.id << "' has no label";
  }
  for (const auto *setting : c.allSettings()) {
    EXPECT_FALSE(setting->groupId.empty()) << "setting '" << setting->key << "' names no group";
    EXPECT_FALSE(setting->label.empty()) << "setting '" << setting->key << "' has no label";
  }
}

// UiSoundPlayer reads this key for interface loudness. It is deliberately not
// the game volume, so the two are pinned separately
TEST(ShippedSettingsCatalogTest, DeclaresTheUiSoundVolumeKey) {
  SettingsCatalog c;
  ASSERT_TRUE(c.loadFromFile(FL_SETTINGS_CATALOG_FILE));

  const auto *setting = c.findByKey(firelight::audio::UI_SOUND_VOLUME_KEY);
  ASSERT_NE(setting, nullptr) << "UiSoundPlayer reads '" << firelight::audio::UI_SOUND_VOLUME_KEY
                              << "', which the catalog doesn't declare";
  EXPECT_TRUE(c.isAppSetting(firelight::audio::UI_SOUND_VOLUME_KEY)) << "interface volume is read from the global tier";
  EXPECT_EQ(setting->type, SettingType::INTEGER);
  EXPECT_EQ(setting->widget, "slider");
  EXPECT_EQ(setting->minValue, 0);
  EXPECT_EQ(setting->maxValue, 100);
  EXPECT_EQ(setting->defaultValue, "100");
  // Sharing the game volume's key would mean quietening a game also silenced
  // menu feedback, which is the thing this key exists to avoid
  EXPECT_STRNE(firelight::audio::UI_SOUND_VOLUME_KEY, firelight::audio::VOLUME_KEY);
}

// AudioManager reads this key to pick an output. If it stopped existing, audio
// would quietly fall back to the system default with no error anywhere — so the
// key and its declaration are pinned together
TEST(ShippedSettingsCatalogTest, DeclaresTheAudioOutputKeyAudioManagerReads) {
  SettingsCatalog c;
  ASSERT_TRUE(c.loadFromFile(FL_SETTINGS_CATALOG_FILE));

  const auto *setting = c.findByKey(AudioManager::OUTPUT_DEVICE_KEY);
  ASSERT_NE(setting, nullptr) << "AudioManager reads '" << AudioManager::OUTPUT_DEVICE_KEY
                              << "', which the catalog doesn't declare";
  EXPECT_TRUE(c.isAppSetting(AudioManager::OUTPUT_DEVICE_KEY)) << "the output device is read from the global tier";
  // Its options come from the machine, so authoring any here would be a lie —
  // and the runtime flag is what tells the model to go and find them
  EXPECT_TRUE(setting->audioDeviceSource);
  EXPECT_TRUE(setting->options.empty());
  // "" is the system default; anything else names a device
  EXPECT_TRUE(setting->defaultValue.empty());
}

// Mute lives in the catalog rather than on the emulator so it outlives a game:
// AudioManager is rebuilt on every load and re-reads this key, which is the only
// reason muting one game leaves the next one muted too
TEST(ShippedSettingsCatalogTest, DeclaresTheMuteKeyAudioManagerReads) {
  SettingsCatalog c;
  ASSERT_TRUE(c.loadFromFile(FL_SETTINGS_CATALOG_FILE));

  const auto *setting = c.findByKey(AudioManager::MUTED_KEY);
  ASSERT_NE(setting, nullptr) << "AudioManager reads '" << AudioManager::MUTED_KEY
                              << "', which the catalog doesn't declare";
  EXPECT_TRUE(c.isAppSetting(AudioManager::MUTED_KEY)) << "mute is read from the global tier";
  EXPECT_EQ(setting->type, SettingType::BOOLEAN);
  // AudioManager compares against "true"/"false", so the declared default has to
  // be one of them
  EXPECT_EQ(setting->defaultValue, "false");
}

// The volume hotkeys and AudioManager both read this key, and neither would say
// anything if it went missing — the slider would just stop doing anything
TEST(ShippedSettingsCatalogTest, DeclaresTheVolumeKeyAudioManagerReads) {
  SettingsCatalog c;
  ASSERT_TRUE(c.loadFromFile(FL_SETTINGS_CATALOG_FILE));

  const auto *setting = c.findByKey(AudioManager::VOLUME_KEY);
  ASSERT_NE(setting, nullptr) << "AudioManager reads '" << AudioManager::VOLUME_KEY
                              << "', which the catalog doesn't declare";
  EXPECT_TRUE(c.isAppSetting(AudioManager::VOLUME_KEY)) << "volume is read from the global tier";
  EXPECT_EQ(setting->type, SettingType::INTEGER);
  EXPECT_EQ(setting->widget, "slider");
  // The hotkeys step within 0-100 and parse the default as a number; a range
  // that disagreed would let the slider and the hotkey mean different things
  EXPECT_EQ(setting->minValue, 0);
  EXPECT_EQ(setting->maxValue, 100);
  EXPECT_EQ(setting->defaultValue, "100");
}

// AppearanceSettings.qml is a typed facade: one SettingBinding per key, so the
// rest of the app can bind to names instead of strings. Nothing in QML fails
// loudly when a key stops existing — the binding just reports "" forever — so
// pin the keys here
TEST(ShippedSettingsCatalogTest, DeclaresEveryKeyTheAppearanceFacadeBinds) {
  SettingsCatalog c;
  ASSERT_TRUE(c.loadFromFile(FL_SETTINGS_CATALOG_FILE));

  const std::vector<std::string> facadeKeys = {"accent-color",       "background-mode", "background-color",
                                               "background-color-2", "background-file", "background-blur",
                                               "background-dim",     "theme-intensity", "glass-opacity",
                                               "library-tile-size",  "interface-scale", "interface-density"};

  for (const auto &key : facadeKeys) {
    const auto *setting = c.findByKey(key);
    EXPECT_NE(setting, nullptr) << "AppearanceSettings.qml binds '" << key << "', which the catalog no longer declares";
    if (setting == nullptr) {
      continue;
    }
    EXPECT_TRUE(c.isAppSetting(key)) << "'" << key << "' must be an app setting: the facade reads the global tier only";
    // An empty file path means "no image chosen"; every other facade key needs
    // a default or the property starts empty and the theme renders wrong
    if (key != "background-file") {
      EXPECT_FALSE(setting->defaultValue.empty()) << "'" << key << "' has no default, so the facade would start empty";
    }
  }
}

} // namespace firelight::settings

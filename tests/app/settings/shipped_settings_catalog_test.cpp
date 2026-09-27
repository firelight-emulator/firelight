#include "audio/audio_settings.hpp"

#include <firelight/settings/settings_catalog.hpp>

#include <gtest/gtest.h>
#include <string>
#include <vector>

namespace firelight::settings {

TEST(ShippedSettingsCatalogTest, ParsesAndValidatesCleanly) {
  SettingsCatalog c;
  ASSERT_TRUE(c.loadFromDirectory(FL_SETTINGS_CATALOG_DIR)) << "could not read " << FL_SETTINGS_CATALOG_DIR;

  const auto problems = c.validate();
  for (const auto &problem : problems) {
    ADD_FAILURE() << "settings_catalog.json: " << problem;
  }

  EXPECT_FALSE(c.pages().empty());
  EXPECT_FALSE(c.groups().empty());
  EXPECT_FALSE(c.commonSettings().empty());

  for (const auto &group : c.groups()) {
    EXPECT_FALSE(group.label.empty()) << "group '" << group.id << "' has no label";
  }

  for (const auto *setting : c.allSettings()) {
    EXPECT_FALSE(setting->label.empty()) << "setting '" << setting->key << "' has no label";
  }
}

TEST(ShippedSettingsCatalogTest, DeclaresTheUiSoundVolumeKey) {
  SettingsCatalog c;
  ASSERT_TRUE(c.loadFromDirectory(FL_SETTINGS_CATALOG_DIR));

  const auto *setting = c.findByKey(audio::UI_SOUND_VOLUME_KEY);
  ASSERT_NE(setting, nullptr) << "UiSoundPlayer reads '" << audio::UI_SOUND_VOLUME_KEY
                              << "', which the catalog doesn't declare";
  EXPECT_TRUE(c.isAppSetting(firelight::audio::UI_SOUND_VOLUME_KEY)) << "interface volume is read from the global tier";
  EXPECT_EQ(setting->type, SettingType::INTEGER);
  EXPECT_EQ(setting->widget, "slider");
  EXPECT_EQ(setting->minValue, 0);
  EXPECT_EQ(setting->maxValue, 100);
  EXPECT_EQ(setting->defaultValue, "100");

  EXPECT_STRNE(firelight::audio::UI_SOUND_VOLUME_KEY, firelight::audio::VOLUME_KEY);
}

TEST(ShippedSettingsCatalogTest, DeclaresTheAudioOutputKeyAudioManagerReads) {
  SettingsCatalog c;
  ASSERT_TRUE(c.loadFromDirectory(FL_SETTINGS_CATALOG_DIR));

  const auto *setting = c.findByKey(audio::OUTPUT_DEVICE_KEY);
  ASSERT_NE(setting, nullptr) << "AudioManager reads '" << audio::OUTPUT_DEVICE_KEY
                              << "', which the catalog doesn't declare";
  EXPECT_TRUE(c.isAppSetting(audio::OUTPUT_DEVICE_KEY)) << "the output device is read from the global tier";

  EXPECT_TRUE(setting->audioDeviceSource);
  EXPECT_TRUE(setting->options.empty());

  // "" is the system default; anything else names a device
  EXPECT_TRUE(setting->defaultValue.empty());
}

TEST(ShippedSettingsCatalogTest, DeclaresTheMuteKeyAudioManagerReads) {
  SettingsCatalog c;
  ASSERT_TRUE(c.loadFromDirectory(FL_SETTINGS_CATALOG_DIR));

  const auto *setting = c.findByKey(audio::MUTED_KEY);
  ASSERT_NE(setting, nullptr) << "AudioManager reads '" << audio::MUTED_KEY << "', which the catalog doesn't declare";
  EXPECT_TRUE(c.isAppSetting(audio::MUTED_KEY)) << "mute is read from the global tier";
  EXPECT_EQ(setting->type, SettingType::BOOLEAN);

  EXPECT_EQ(setting->defaultValue, "false");
}

// The volume hotkeys and AudioManager both read this key, and if it were missing the slider would just stop doing
// anything
TEST(ShippedSettingsCatalogTest, DeclaresTheVolumeKeyAudioManagerReads) {
  SettingsCatalog c;
  ASSERT_TRUE(c.loadFromDirectory(FL_SETTINGS_CATALOG_DIR));

  const auto *setting = c.findByKey(audio::VOLUME_KEY);
  ASSERT_NE(setting, nullptr) << "AudioManager reads '" << audio::VOLUME_KEY << "', which the catalog doesn't declare";
  EXPECT_TRUE(c.isAppSetting(audio::VOLUME_KEY)) << "volume is read from the global tier";
  EXPECT_EQ(setting->type, SettingType::INTEGER);
  EXPECT_EQ(setting->widget, "slider");

  EXPECT_EQ(setting->minValue, 0);
  EXPECT_EQ(setting->maxValue, 100);
  EXPECT_EQ(setting->defaultValue, "100");
}

TEST(ShippedSettingsCatalogTest, DeclaresEveryKeyTheAppearanceFacadeBinds) {
  SettingsCatalog c;
  ASSERT_TRUE(c.loadFromDirectory(FL_SETTINGS_CATALOG_DIR));

  const std::vector<std::string> facadeKeys = {"accent-color",       "background-mode", "background-color",
                                               "background-color-2", "background-file", "background-blur",
                                               "background-dim",     "theme-intensity", "glass-opacity",
                                               "library-icon-grid-tile-size",  "interface-scale", "interface-density"};

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

TEST(ShippedSettingsCatalogTest, DeclaresEveryKeyTheGeneralFacadeBinds) {
  SettingsCatalog c;
  ASSERT_TRUE(c.loadFromDirectory(FL_SETTINGS_CATALOG_DIR));

  const std::vector<std::string> facadeKeys = {"fullscreen", "show-advanced-settings", "show-new-user-flow"};

  for (const auto &key : facadeKeys) {
    const auto *setting = c.findByKey(key);
    EXPECT_NE(setting, nullptr) << "GeneralSettings.qml binds '" << key << "', which the catalog no longer declares";
    if (setting == nullptr) {
      continue;
    }

    EXPECT_TRUE(c.isAppSetting(key)) << "'" << key << "' must be an app setting: the facade reads the global tier only";
    EXPECT_FALSE(setting->defaultValue.empty()) << "'" << key << "' has no default, so the facade would start empty";
  }
}

TEST(ShippedSettingsCatalogTest, DeclaresTheVariantOrderingKeys) {
  SettingsCatalog c;
  ASSERT_TRUE(c.loadFromDirectory(FL_SETTINGS_CATALOG_DIR));

  const auto *regions = c.findByKey("library-region-priority");
  ASSERT_NE(regions, nullptr);
  EXPECT_FALSE(regions->defaultValue.empty());

  const auto *languages = c.findByKey("library-language-priority");
  ASSERT_NE(languages, nullptr);
  EXPECT_FALSE(languages->defaultValue.empty());
}

} // namespace firelight::settings

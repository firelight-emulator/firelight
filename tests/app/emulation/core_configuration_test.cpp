#include <libretro/core_configuration.hpp>

#include <firelight/settings/setting_definition.hpp>
#include <firelight/settings/settings_service.hpp>
#include <firelight/settings/sqlite_settings_repository.hpp>

#include <gtest/gtest.h>

namespace {

using firelight::settings::CoreOptionMapping;
using firelight::settings::SettingDefinition;

class CoreConfigurationTest : public testing::Test {
protected:
  firelight::settings::SqliteSettingsRepository m_repo{":memory:"};
  firelight::settings::SettingsService m_service{m_repo};
  const int m_platformId = 42;
  const std::string m_hash = "hash1";

  static firelight::libretro::IConfigurationProvider::Option
  coreOption(const std::string &key, const std::string &def) {
    firelight::libretro::IConfigurationProvider::Option o;
    o.key = key;
    o.defaultValueKey = def;
    o.possibleValues = {{.key = def, .label = def}};
    return o;
  }
};

TEST_F(CoreConfigurationTest, GetOptionsReturnsRegisteredOptions) {
  CoreConfiguration config(m_hash, m_platformId, {}, {}, m_service);
  config.registerOption(coreOption("core_a", "a1"));
  config.registerOption(coreOption("core_b", "b1"));

  const auto options = config.getOptions();
  EXPECT_EQ(options.size(), 2u);
}

TEST_F(CoreConfigurationTest, FallsBackToCoreDeclaredDefault) {
  CoreConfiguration config(m_hash, m_platformId, {}, {}, m_service);
  config.registerOption(coreOption("core_x", "coredefault"));
  EXPECT_EQ(config.getOptionValue("core_x").value_or(""), "coredefault");
}

TEST_F(CoreConfigurationTest, CatalogCoreDefaultBeatsCoreDeclaredDefault) {
  CoreConfiguration config(m_hash, m_platformId, {},
                           {{"core_x", "catalogdefault"}}, m_service);
  config.registerOption(coreOption("core_x", "coredefault"));
  EXPECT_EQ(config.getOptionValue("core_x").value_or(""), "catalogdefault");
}

TEST_F(CoreConfigurationTest, FriendlyMappingDrivesCoreValue) {
  SettingDefinition s;
  s.key = "friendly-color";
  s.defaultValue = "Accurate";
  s.mapping = {{.coreKey = "core_color",
                .valueMap = {{"Accurate", "accurate"}, {"Fast", "fast"}}}};

  CoreConfiguration config(m_hash, m_platformId, {s}, {}, m_service);
  config.registerOption(coreOption("core_color", "coredefault"));

  // No user override -> friendly default "Accurate" -> mapped "accurate"
  EXPECT_EQ(config.getOptionValue("core_color").value_or(""), "accurate");

  // Set the friendly setting at platform level -> mapped "fast"
  m_service.setPlatformValue(m_platformId, "friendly-color", "Fast");
  EXPECT_EQ(config.getOptionValue("core_color").value_or(""), "fast");
}

TEST_F(CoreConfigurationTest, AdvancedRawOverrideBeatsFriendlyMapping) {
  SettingDefinition s;
  s.key = "friendly-color";
  s.defaultValue = "Accurate";
  s.mapping = {{.coreKey = "core_color", .valueMap = {{"Accurate", "accurate"}}}};

  CoreConfiguration config(m_hash, m_platformId, {s}, {}, m_service);
  config.registerOption(coreOption("core_color", "coredefault"));

  // A raw override of the core key (advanced editor) wins over the friendly map
  m_service.setGameValue(m_hash, "core_color", "raw-advanced");
  EXPECT_EQ(config.getOptionValue("core_color").value_or(""), "raw-advanced");
}

TEST_F(CoreConfigurationTest, OverridesResolveGameOverPlatformOverGlobal) {
  CoreConfiguration config(m_hash, m_platformId, {}, {}, m_service);
  config.registerOption(coreOption("core_x", "coredefault"));

  m_service.setGlobalValue("core_x", "globalval");
  EXPECT_EQ(config.getOptionValue("core_x").value_or(""), "globalval");

  m_service.setPlatformValue(m_platformId, "core_x", "platval");
  EXPECT_EQ(config.getOptionValue("core_x").value_or(""), "platval");

  m_service.setGameValue(m_hash, "core_x", "gameval");
  EXPECT_EQ(config.getOptionValue("core_x").value_or(""), "gameval");
}

// A CLI --set targeting a raw core option key flows through getEffectiveValue,
// so it wins over every stored tier here too (proving one session-override
// mechanism covers both common settings and per-core options)
TEST_F(CoreConfigurationTest, SessionOverrideOnCoreKeyWins) {
  CoreConfiguration config(m_hash, m_platformId, {}, {}, m_service);
  config.registerOption(coreOption("core_x", "coredefault"));

  m_service.setGameValue(m_hash, "core_x", "gameval");
  EXPECT_EQ(config.getOptionValue("core_x").value_or(""), "gameval");

  m_service.setSessionOverride("core_x", "sessionval");
  EXPECT_EQ(config.getOptionValue("core_x").value_or(""), "sessionval");
}

} // namespace

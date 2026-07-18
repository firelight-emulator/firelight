#pragma once
#include <firelight/event_dispatcher.hpp>
#include <firelight/libretro/configuration_provider.hpp>
#include <firelight/settings/setting_definition.hpp>
#include <firelight/settings/settings_service.hpp>
#include <map>
#include <string>
#include <vector>

class CoreConfiguration final
    : public firelight::libretro::IConfigurationProvider {
public:
  // Friendly settings + Firelight core-option default overrides are resolved
  // from the settings catalog (by core) and injected here, so this class is
  // decoupled from Platform / the catalog itself
  CoreConfiguration(
      std::string contentHash, int platformId,
      std::vector<firelight::settings::SettingDefinition> friendlySettings,
      std::map<std::string, std::string> coreDefaults,
      firelight::settings::SettingsService &settingsService);

  ~CoreConfiguration() = default;

  void registerOption(Option option) override;

  [[nodiscard]] std::vector<Option> getOptions() const override;

  bool anyOptionValueHasChanged() override;

  void setDefaultValue(const std::string &key,
                       const std::string &value) override;

  std::optional<std::string> getOptionValue(const std::string &key) override;

  void setOptionVisibility(const std::string &key, bool visible) override;

  void setPlatformValue(const std::string &key, const std::string &value);

  void setGameValue(const std::string &key, const std::string &value);

private:
  std::string m_contentHash;
  int m_platformId;
  std::vector<firelight::settings::SettingDefinition> m_friendlySettings;
  std::map<std::string, std::string> m_coreDefaults;
  firelight::settings::SettingsService &m_settingsService;

  std::map<std::string, std::string> m_cache;
  bool m_anyValueHasChanged = false;

  ScopedConnection m_platformSettingChangedConnection;
  ScopedConnection m_gameSettingChangedConnection;
  ScopedConnection m_levelChangedConnection;

  std::map<std::string, Option> m_options;
  bool m_changedSinceLastChecked = false;

  std::map<std::string, std::string> m_defaultValues;
  std::map<std::string, std::string> m_platformValues;
  std::map<std::string, std::string> m_gameValues;
};

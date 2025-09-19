#pragma once
#include <event_dispatcher.hpp>
#include <firelight/libretro/configuration_provider.hpp>
#include <map>
#include <platforms/models/platform.hpp>
#include <settings/settings_service.hpp>

class CoreConfiguration final
    : public firelight::libretro::IConfigurationProvider {
public:
  explicit CoreConfiguration(
      std::string contentHash, firelight::platforms::Platform platform,
      firelight::settings::SettingsService &settingsService);

  virtual ~CoreConfiguration() = default;

  void registerOption(Option option) override;

  bool anyOptionValueHasChanged() override;

  void setDefaultValue(const std::string &key,
                       const std::string &value) override;

  std::optional<std::string> getOptionValue(const std::string &key) override;

  void setOptionVisibility(const std::string &key, bool visible) override;

  void setPlatformValue(const std::string &key, const std::string &value);

  void setGameValue(const std::string &key, const std::string &value);

private:
  std::string m_contentHash;
  firelight::platforms::Platform m_platform;
  firelight::settings::SettingsService &m_settingsService;
  firelight::settings::SettingsLevel m_settingsLevel;

  std::map<std::string, std::string> m_cache;
  // std::map<std::string, bool> m_changedFlags;
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

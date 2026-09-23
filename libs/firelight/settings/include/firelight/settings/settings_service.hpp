#pragma once

#include <firelight/settings/settings_repository.hpp>

#include <map>
#include <optional>
#include <string>

namespace firelight::settings {

// Don't change these!!!
enum SettingsLevel { Game, Platform, Global, Unknown };

struct GameSettingChangedEvent {
  std::string contentHash;
  std::string key;
  std::string value;
};

struct GameSettingResetEvent {
  std::string contentHash;
  std::string key;
};

struct PlatformSettingChangedEvent {
  int platformId;
  std::string key;
  std::string value;
};

struct PlatformSettingResetEvent {
  int platformId;
  std::string key;
};

struct GlobalSettingChangedEvent {
  std::string key;
  std::string value;
};

struct GlobalSettingResetEvent {
  std::string key;
};

struct EmulationSettingChangedEvent {
  std::string contentHash;
  std::string key;
};

class SettingsService {
public:
  explicit SettingsService(ISettingsRepository &settingsRepo);
  ~SettingsService() = default;

  static void setInstance(SettingsService *instance) { s_instance = instance; }

  static SettingsService *instance() { return s_instance; }

  std::optional<std::string> getGlobalValue(const std::string &key);
  bool setGlobalValue(const std::string &key, const std::string &value);
  bool resetGlobalValue(const std::string &key);

  std::optional<std::string> getPlatformValue(int platformId, const std::string &key);
  bool setPlatformValue(int platformId, const std::string &key, const std::string &value);
  bool resetPlatformValue(int platformId, const std::string &key);

  std::optional<std::string> getGameValue(const std::string &contentHash, const std::string &key);
  bool setGameValue(const std::string &contentHash, const std::string &key, const std::string &value);
  bool resetGameValue(const std::string &contentHash, const std::string &key);

  bool setValueAtLevel(SettingsLevel level, const std::string &contentHash, int platformId, const std::string &key,
                       const std::string &value);

  std::optional<std::string> getValueAtLevel(SettingsLevel level, const std::string &contentHash, int platformId,
                                             const std::string &key);
  bool resetValueAtLevel(SettingsLevel level, const std::string &contentHash, int platformId, const std::string &key);

  // session override -> game -> platform -> global
  std::optional<std::string> getEffectiveValue(const std::string &contentHash, int platformId, const std::string &key);

  // session override -> global for app settings and stuff
  std::optional<std::string> getGlobalEffectiveValue(const std::string &key);

  // Sets an in-memory, non-persisted override that wins over every stored tier
  // in getEffectiveValue. Used by CLI
  void setSessionOverride(const std::string &key, const std::string &value);
  void clearSessionOverrides();

private:
  static SettingsService *s_instance;
  ISettingsRepository &m_settingsRepo;
  std::map<std::string, std::string> m_sessionOverrides;
};

} // namespace firelight::settings

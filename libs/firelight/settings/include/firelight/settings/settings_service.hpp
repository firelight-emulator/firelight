#pragma once

#include <firelight/settings/settings_repository.hpp>
#include <optional>
#include <string>

namespace firelight::settings {

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

  std::optional<std::string> getGameValue(const std::string &contentHash,
                                          const std::string &key);
  bool setGameValue(const std::string &contentHash, const std::string &key,
                    const std::string &value);
  bool resetGameValue(const std::string &contentHash, const std::string &key);

  bool setValueAtLevel(SettingsLevel level, const std::string &contentHash,
                       int platformId, const std::string &key,
                       const std::string &value);
  // Reads the value stored *at exactly* `level` (no fallback). Used to tell
  // whether a level has its own override. Contrast with getEffectiveValue.
  std::optional<std::string> getValueAtLevel(SettingsLevel level,
                                             const std::string &contentHash,
                                             int platformId,
                                             const std::string &key);
  // Clears the override at `level` so the setting falls through to the next tier.
  bool resetValueAtLevel(SettingsLevel level, const std::string &contentHash,
                         int platformId, const std::string &key);

  // Canonical resolution: game override -> platform override -> global. Returns
  // nullopt if unset at every level (caller applies the catalog default).
  std::optional<std::string> getEffectiveValue(const std::string &contentHash,
                                               int platformId,
                                               const std::string &key);

private:
  static SettingsService *s_instance;
  ISettingsRepository &m_settingsRepo;
};

} // namespace firelight::settings

#pragma once
#include "settings_repository.hpp"

#include <optional>
#include <string>

namespace firelight::settings {

struct SettingsLevelChangedEvent {
  std::string contentHash;
  SettingsLevel level;
};

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

class SettingsService {
public:
  explicit SettingsService(ISettingsRepository &settingsRepo);
  ~SettingsService() = default;

  SettingsLevel getSettingsLevel(std::string contentHash);
  bool setSettingsLevel(std::string contentHash, SettingsLevel level);

  std::optional<std::string> getPlatformValue(int platformId,
                                              const std::string &key);

  bool setPlatformValue(int platformId, const std::string &key,
                        const std::string &value);

  bool resetPlatformValue(int platformId, const std::string &key);

  std::optional<std::string> getGameValue(const std::string &contentHash,
                                          const std::string &key);

  bool setGameValue(const std::string &contentHash, const std::string &key,
                    const std::string &value);

  bool resetGameValue(const std::string &contentHash, const std::string &key);

private:
  ISettingsRepository &m_settingsRepo;
};

} // namespace firelight::settings
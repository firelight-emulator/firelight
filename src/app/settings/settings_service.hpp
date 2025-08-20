#pragma once
#include "settings_repository.hpp"

#include <optional>
#include <string>

namespace firelight::settings {

enum SettingsLevel { Game, Platform };

struct SettingsLevelChangedEvent {
  std::string contentHash;
  SettingsLevel level;
};

struct GameSettingChangedEvent {
  std::string contentHash;
  std::string key;
  std::string value;
};

struct PlatformSettingChangedEvent {
  int platformId;
  std::string key;
  std::string value;
};

class SettingsService {
public:
  explicit SettingsService(ISettingsRepository &settingsRepo);
  ~SettingsService() = default;
  SettingsLevel getSettingsLevel(std::string contentHash);
  void setSettingsLevel(std::string contentHash, SettingsLevel level);

  std::optional<std::string> getPlatformValue(int platformId,
                                              const std::string &key);

  void setPlatformValue(int platformId, const std::string &key,
                        const std::string &value);

  void resetPlatformValue(int platformId, const std::string &key);

  std::optional<std::string> getGameValue(const std::string &contentHash,
                                          const std::string &key);

  void setGameValue(const std::string &contentHash, const std::string &key,
                    const std::string &value);

  void resetGameValue(const std::string &contentHash, const std::string &key);

private:
  ISettingsRepository &m_settingsRepo;
};

} // namespace firelight::settings
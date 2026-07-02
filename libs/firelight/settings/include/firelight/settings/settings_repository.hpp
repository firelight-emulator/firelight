#pragma once

#include <optional>
#include <string>

namespace firelight::settings {

// Order matters: Game/Platform keep their historic ints (stored in
// game_setting_levels), Global is appended so existing rows are unaffected.
enum SettingsLevel { Game, Platform, Global, Unknown };

class ISettingsRepository {
public:
  virtual ~ISettingsRepository() = default;

  virtual SettingsLevel getSettingsLevel(std::string contentHash) = 0;
  virtual bool setSettingsLevel(std::string contentHash, SettingsLevel level) = 0;

  virtual std::optional<std::string> getGlobalValue(const std::string &key) = 0;
  virtual bool setGlobalValue(const std::string &key,
                              const std::string &value) = 0;
  virtual bool resetGlobalValue(const std::string &key) = 0;

  virtual std::optional<std::string> getPlatformValue(int platformId,
                                                       const std::string &key) = 0;
  virtual bool setPlatformValue(int platformId, const std::string &key,
                                const std::string &value) = 0;
  virtual bool resetPlatformValue(int platformId, const std::string &key) = 0;

  virtual std::optional<std::string> getGameValue(const std::string &contentHash,
                                                   const std::string &key) = 0;
  virtual bool setGameValue(const std::string &contentHash, const std::string &key,
                            const std::string &value) = 0;
  virtual bool resetGameValue(const std::string &contentHash,
                              const std::string &key) = 0;
};

} // namespace firelight::settings

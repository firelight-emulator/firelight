#pragma once

#include <optional>
#include <string>

namespace firelight::settings {

// A setting override tier. Values resolve Game -> Platform -> Global -> default;
// there is no stored "current level" per game — inheritance is the fallback
// chain. `Unknown` is a sentinel for "not a real tier".
enum SettingsLevel { Game, Platform, Global, Unknown };

class ISettingsRepository {
public:
  virtual ~ISettingsRepository() = default;

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

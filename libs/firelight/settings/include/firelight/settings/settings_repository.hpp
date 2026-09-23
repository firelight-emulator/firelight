#pragma once

#include <optional>
#include <string>

namespace firelight::settings {

// Storage of setting values, per level. The service handles stuff like getEffectiveValue()
class ISettingsRepository {
public:
  virtual ~ISettingsRepository() = default;

  virtual std::optional<std::string> getGlobalValue(const std::string &key) = 0;
  virtual bool setGlobalValue(const std::string &key, const std::string &value) = 0;
  virtual bool resetGlobalValue(const std::string &key) = 0;

  virtual std::optional<std::string> getPlatformValue(int platformId, const std::string &key) = 0;
  virtual bool setPlatformValue(int platformId, const std::string &key, const std::string &value) = 0;
  virtual bool resetPlatformValue(int platformId, const std::string &key) = 0;

  virtual std::optional<std::string> getGameValue(const std::string &contentHash, const std::string &key) = 0;
  virtual bool setGameValue(const std::string &contentHash, const std::string &key, const std::string &value) = 0;
  virtual bool resetGameValue(const std::string &contentHash, const std::string &key) = 0;
};

} // namespace firelight::settings

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

  /** A controller profile's stored value, or its value for one game when contentHash is not empty */
  virtual std::optional<std::string> getControllerValue(const std::string &contentHash, int profileId,
                                                        const std::string &key) = 0;

  /** Stores a controller profile's value, or its value for one game when contentHash is not empty */
  virtual bool setControllerValue(const std::string &contentHash, int profileId, const std::string &key,
                                  const std::string &value) = 0;

  /** Removes a controller profile's stored value, or its value for one game when contentHash is not empty */
  virtual bool resetControllerValue(const std::string &contentHash, int profileId, const std::string &key) = 0;
};

} // namespace firelight::settings

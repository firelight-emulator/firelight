#pragma once

#include <functional>
#include <optional>
#include <string>

namespace firelight::settings {
class IEmulationSettingsManager {
public:
  virtual ~IEmulationSettingsManager() = default;

  virtual std::optional<std::string> getGlobalValue(const std::string &key) = 0;
  virtual std::optional<std::string>
  getPlatformValue(int platformId, const std::string &key) = 0;
  virtual std::optional<std::string>
  getGameValue(const std::string &contentHash, int platformId,
               const std::string &key) = 0;

  virtual void setGlobalValue(const std::string &key,
                              const std::string &value) = 0;
  virtual void setPlatformValue(int platformId, const std::string &key,
                                const std::string &value) = 0;
  virtual void setGameValue(const std::string &contentHash, int platformId,
                            const std::string &key,
                            const std::string &value) = 0;

  virtual void resetGlobalValue(const std::string &key) = 0;
  virtual void resetPlatformValue(int platformId, const std::string &key) = 0;
  virtual void resetGameValue(const std::string &contentHash, int platformId,
                              const std::string &key) = 0;

  virtual void setOnGlobalValueChanged(
      std::function<void(const std::string &key, const std::string &value)>
          callback) = 0;
  virtual void setOnPlatformValueChanged(
      std::function<void(int platformId, const std::string &key,
                         const std::string &value)>
          callback) = 0;
  virtual void setOnGameValueChanged(
      std::function<void(const std::string &contentHash, int platformId,
                         const std::string &key, const std::string &value)>
          callback) = 0;
};
} // namespace firelight::settings
#pragma once

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

  virtual std::string getEffectiveValue(const std::string &contentHash,
                                        int platformId, const std::string &key,
                                        const std::string &defaultValue) = 0;

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
};
} // namespace firelight::settings
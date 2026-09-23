#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace libretro {
class ICore;
}

namespace firelight::achievements {

class IAchievementClient {
public:
  virtual ~IAchievementClient() = default;

  virtual void loadGame(int platformId, const std::string &contentMd5) = 0;
  virtual void doFrame(::libretro::ICore *core) = 0;
  virtual void reset() = 0;
  [[nodiscard]] virtual bool loggedIn() const = 0;
  [[nodiscard]] virtual bool hardcoreModeActive() const = 0;
  virtual std::vector<uint8_t> serializeState() { return {}; }
  virtual void deserializeState(const std::vector<uint8_t> &state) {}
};

} // namespace firelight::achievements

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace libretro {
class ICore;
}

namespace firelight::achievements {

// The slice of the RetroAchievements client the emulation loop drives, so the
// emulation layer depends on this interface instead of the Qt/GUI-bound RAClient
class IAchievementClient {
public:
  virtual ~IAchievementClient() = default;

  virtual void loadGame(int platformId, const std::string &contentMd5) = 0;
  virtual void doFrame(::libretro::ICore *core) = 0;
  virtual void reset() = 0;
  [[nodiscard]] virtual bool loggedIn() const = 0;
  [[nodiscard]] virtual bool hardcoreModeActive() const = 0;

  // A save state that restored the game but not the achievement session would put the player back
  // somewhere the client has never been. Defaulted, because a client with nothing worth restoring is
  // a legitimate one
  virtual std::vector<uint8_t> serializeState() { return {}; }

  virtual void deserializeState(const std::vector<uint8_t> &state) {}
};

} // namespace firelight::achievements

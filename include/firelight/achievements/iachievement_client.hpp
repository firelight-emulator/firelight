#pragma once

#include <string>

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
};

} // namespace firelight::achievements

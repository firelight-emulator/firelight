#pragma once
#include <functional>
#include <memory>
#include <string>

class IAudioOutput;

namespace firelight::libretro {
class IAudioInputProvider;
class IRetropadProvider;
} // namespace firelight::libretro

namespace firelight::media {
class IClipSink;
}

namespace firelight::input {
class InputService;
}

namespace firelight::achievements {
class IAchievementClient;
}

namespace firelight::saves {
class ISaveManager;
}

namespace firelight::settings {
class ICoreOptionRepository;
class SettingsService;
} // namespace firelight::settings

namespace firelight::cheats {
class ICheatRepository;
}

namespace firelight::platforms {
class IPlatformService;
}

namespace firelight::emulation {

// The app-layer services EmulationService/EmulatorInstance depend on, threaded
// explicitly from main.cpp instead of reached through a global locator. Any
// member may be null in tests that don't exercise that path — the consumers
// null-guard the optional services (input, achievements, save manager)
struct EmulationContext {
  input::InputService *inputService = nullptr;
  achievements::IAchievementClient *achievementManager = nullptr;
  saves::ISaveManager *saveManager = nullptr;
  settings::SettingsService *settingsService = nullptr;
  settings::ICoreOptionRepository *coreOptionRepository = nullptr;
  cheats::ICheatRepository *cheatRepository = nullptr;
  platforms::IPlatformService *platformService = nullptr;

  // TODO
  // Rebuilds the playlist a disc set launches through, called with the set and the identity it
  // launches under before the core is handed the path. Unset means the path is used as-is
  std::function<void(int, const std::string &)> materializePlaylist;

  std::string coreSystemDirectory;

  // Netplay seams. retropadProvider (when set) answers the core's per-port
  // reads instead of inputService directly, so remote players can occupy
  // ports. netplayStreamSink receives every rendered frame + audio while a
  // host stream is armed (a cheap no-op otherwise)
  libretro::IRetropadProvider *retropadProvider = nullptr;
  media::IClipSink *netplayStreamSink = nullptr;

  // Audio output + microphone are created on the render thread inside
  // EmulatorInstance::initialize(); main.cpp injects the Qt-Multimedia impls
  // (AudioManager / QtMicrophone). Null in headless/tests -> no audio
  std::function<std::shared_ptr<IAudioOutput>()> audioOutputFactory;
  std::function<std::unique_ptr<libretro::IAudioInputProvider>()> audioInputFactory;
};

} // namespace firelight::emulation

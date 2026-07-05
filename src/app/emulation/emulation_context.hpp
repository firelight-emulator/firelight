#pragma once
#include <string>

namespace firelight::input {
class InputService;
}
namespace firelight::achievements {
class RAClient;
}
namespace firelight::saves {
class ISaveManager;
}
namespace firelight::settings {
class ICoreOptionRepository;
class SettingsService;
}
namespace firelight::cheats {
class ICheatRepository;
}

namespace firelight::emulation {

// The app-layer services EmulationService/EmulatorInstance depend on, threaded
// explicitly from main.cpp instead of reached through a global locator. Any
// member may be null in tests that don't exercise that path — the consumers
// null-guard the optional services (input, achievements, save manager).
struct EmulationContext {
  input::InputService *inputService = nullptr;
  achievements::RAClient *achievementManager = nullptr;
  saves::ISaveManager *saveManager = nullptr;
  settings::SettingsService *settingsService = nullptr;
  settings::ICoreOptionRepository *coreOptionRepository = nullptr;
  cheats::ICheatRepository *cheatRepository = nullptr;
  std::string coreSystemDirectory;
};

} // namespace firelight::emulation

#pragma once

#include "emulation_service.hpp"

#include <firelight/library/entry.hpp>
#include <firelight/platforms/platform.hpp>
#include <memory>
#include <optional>
#include <string>

class CoreConfiguration;

namespace firelight::library {
class UserLibraryService;
class EntryResolver;
} // namespace firelight::library

namespace firelight::settings {
class SettingsService;
}

namespace firelight::emulation {

// Outcome of a load attempt: a ready EmulatorInstance plus the state the
// EmulationService facade retains (entry/platform/hash + the core config whose
// declared options it caches once the core starts). success == false on any
// failure, with `instance` null.
struct GameLoadResult {
  bool success = false;
  std::unique_ptr<EmulatorInstance> instance;
  std::shared_ptr<CoreConfiguration> coreConfig;
  library::Entry entry;
  std::optional<platforms::Platform> platform;
  std::string contentHash;
};

// The load pipeline extracted from EmulationService: resolve entry -> content +
// patch -> bytes -> save data -> core + config -> instance. EmulationService
// stays the facade (publishes events, holds the current instance).
class GameLoader {
public:
  GameLoader(library::UserLibraryService &library,
             library::EntryResolver &resolver,
             settings::SettingsService &settingsService,
             const EmulationContext &context);

  GameLoadResult load(int entryId, LaunchOverrides launch,
                      const CoreFactory &factory);

private:
  library::UserLibraryService &m_library;
  library::EntryResolver &m_resolver;
  settings::SettingsService &m_settingsService;
  const EmulationContext &m_context;
};

} // namespace firelight::emulation

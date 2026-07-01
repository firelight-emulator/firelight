#pragma once
#include "emulator_instance.hpp"
#include <firelight/platforms/platform.hpp>

#include <firelight/library/entry.hpp>
#include <firelight/libretro/configuration_provider.hpp>
#include <functional>
#include <manager_accessor.hpp>
#include <memory>
#include <string>

namespace firelight::library {
class UserLibraryService;
class EntryResolver;
} // namespace firelight::library

namespace firelight::emulation {

// Builds the ICore for a loaded entry. Injectable so tests can supply a fake
// core instead of dlopen'ing a real libretro DLL. A null factory uses the
// default (real Core).
using CoreFactory = std::function<std::unique_ptr<::libretro::ICore>(
    int platformId, const std::string &corePath,
    std::shared_ptr<firelight::libretro::IConfigurationProvider> configProvider,
    const std::string &systemDirectory)>;

struct GameLoadStarted {};
struct GameLoadedEvent {};
struct GameLoadFailedEvent {};

struct GamePausedChangedEvent {
  bool paused;
};

struct EmulationStartedEvent {
  std::string contentHash;
  int saveSlotNumber;
};

struct EmulationStoppedEvent {};

class EmulationService : public ManagerAccessor {
public:
  static EmulationService *getInstance() { return s_emuServiceInstance; }
  static void setInstance(EmulationService *service) {
    s_emuServiceInstance = service;
  }

  EmulationService(library::UserLibraryService &library,
                   library::EntryResolver &entryResolver,
                   settings::SettingsService &settingsService,
                   CoreFactory coreFactory = nullptr);
  ~EmulationService();

  std::future<EmulatorInstance *> loadEntry(int entryId);
  void stopEmulation();
  EmulatorInstance *getCurrentEmulatorInstance();

  [[nodiscard]] bool isGameRunning() const;

  // Information about the currently running game, if any
  [[nodiscard]] std::optional<std::string> getCurrentGameName() const;
  [[nodiscard]] std::optional<library::Entry> getCurrentEntry();
  [[nodiscard]] std::optional<platforms::Platform> getCurrentPlatform() const;

private:
  static EmulationService *s_emuServiceInstance;

  settings::SettingsService &m_settingsService;
  library::UserLibraryService &m_library;
  library::EntryResolver &m_resolver;
  CoreFactory m_coreFactory;

  std::unique_ptr<EmulatorInstance> m_emulatorInstance;

  library::Entry m_currentEntry;
  std::string m_currentContentHash;
  platforms::Platform m_currentPlatform;
  bool m_gameRunning = false;
};

} // namespace firelight::emulation
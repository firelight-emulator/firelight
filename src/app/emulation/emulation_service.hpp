#pragma once
#include "emulator_instance.hpp"
#include <firelight/platforms/platform.hpp>

#include <firelight/library/entry.hpp>
#include <manager_accessor.hpp>
#include <string>

namespace firelight::library {
class UserLibraryService;
class EntryResolver;
} // namespace firelight::library

namespace firelight::emulation {

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
                   settings::SettingsService &settingsService);
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

  std::unique_ptr<EmulatorInstance> m_emulatorInstance;

  library::Entry m_currentEntry;
  std::string m_currentContentHash;
  platforms::Platform m_currentPlatform;
  bool m_gameRunning = false;
};

} // namespace firelight::emulation
#pragma once
#include "emulation_context.hpp"
#include "emulator_instance.hpp"

#include <firelight/library/entry.hpp>
#include <firelight/libretro/configuration_provider.hpp>
#include <firelight/libretro/core_run_config.hpp>
#include <firelight/platforms/platform.hpp>

#include <functional>
#include <memory>
#include <mutex>
#include <string>

namespace firelight::library {
class UserLibraryService;
class EntryResolver;
} // namespace firelight::library

namespace firelight::settings {
class SettingsService;
}

// TODO: Move out of global namespace
// Resolves core-option values for a loaded entry
class CoreConfiguration;

namespace firelight::emulation {

class GameLoader;

// Builds the ICore for a loaded entry. Injectable so tests can supply a fake core instead of using a real libretro
// DLL. A null factory uses the default (real Core)
using CoreFactory = std::function<std::unique_ptr<::libretro::ICore>(const libretro::CoreRunConfig &config)>;

struct GameLoadStarted {};

struct GameLoadedEvent {};

struct GameLoadFailedEvent {
  std::string reason;
};

/** Published when the game is taken off the foreground or hands it back */
struct GameSuspendedChangedEvent {
  bool suspended;
};

/** Published from the render thread whenever undoing the last suspend-point load becomes possible or not */
struct UndoLoadSuspendPointChangedEvent {
  bool available;
};

struct EmulationStartedEvent {
  std::string contentHash;
  int saveSlotNumber;
};

struct EmulationStoppedEvent {};

struct DiscChangedEvent {
  std::string contentHash;
  unsigned index;
  unsigned count;
};

// TODO: I don't like this
// Published once on load when the core advertises selectable port devices, so
// the input UI can offer a per-port device choice. Query the current instance's
// getControllerDevices() for the details
struct ControllerDevicesEvent {
  std::string contentHash;
};

// One-shot, per-launch knobs applied to the next loadEntry and then consumed (so later launches use their own defaults)
struct LaunchOverrides {
  int saveSlot = -1;  // >= 0 replaces the entry's stored active slot
  bool muted = false; // start the instance muted (born muted in initialize())
};

class EmulationService {
public:
  static EmulationService *getInstance() { return s_emuServiceInstance; }

  static void setInstance(EmulationService *service) { s_emuServiceInstance = service; }

  EmulationService(library::UserLibraryService &library, library::EntryResolver &entryResolver,
                   settings::SettingsService &settingsService, EmulationContext context,
                   CoreFactory coreFactory = nullptr);
  ~EmulationService();

  std::future<EmulatorInstance *> loadEntry(int entryId);
  void stopEmulation();

  /**
   * Queues a reboot of the running game, if there is one
   */
  void resetGame();

  /**
   * Sends the running game to the background so no frames or audio until resume(). Publishes GameSuspendedChangedEvent
   * once
   */
  void suspend();

  /**
   * Hands the game back to the foreground. Publishes GameSuspendedChangedEvent once
   */
  void resume();

  /**
   * @return Whether the running game is suspended in the background
   */
  [[nodiscard]] bool isSuspended() const;

  // TODO: Clean these up
  EmulatorInstance *getCurrentEmulatorInstance() const;
  std::weak_ptr<EmulatorInstance> getCurrentEmulatorInstanceHandle();

  float currentAudioBufferLevel();

  void setCurrentAudioMuted(bool muted);
  bool currentAudioMuted();

  /**
   * Queues work on the running instance, if there is one that has come up.
   *
   * @return Whether there was an instance to take it
   */
  bool submitToCurrentEmulator(const EmulatorCommand &command);

  bool isCurrentEmulatorReady();

  // Sets the one-shot launch knobs applied to (and consumed by) the next loadEntry
  void setPendingLaunchOverrides(LaunchOverrides overrides);

  [[nodiscard]] bool isGameRunning() const;

  // Information about the currently running game, if any
  [[nodiscard]] std::optional<std::string> getCurrentGameName() const;
  [[nodiscard]] std::optional<library::Entry> getCurrentEntry();
  [[nodiscard]] std::optional<platforms::Platform> getCurrentPlatform() const;

private:
  static EmulationService *s_emuServiceInstance;

  settings::SettingsService &m_settingsService;
  EmulationContext m_context;
  CoreFactory m_coreFactory;
  std::unique_ptr<GameLoader> m_loader;

  std::shared_ptr<EmulatorInstance> m_emulatorInstance;

  std::mutex m_instanceMutex;

  // One-shot per-launch knobs (save slot, muted, etc), applied and cleared in loadEntry
  LaunchOverrides m_pendingLaunch;

  library::Entry m_currentEntry;
  std::string m_currentContentHash;
  platforms::Platform m_currentPlatform;
  bool m_gameRunning = false;
  bool m_suspended = false;

  void setSuspended(bool suspended);

  // Hold onto the core config to cache it
  std::shared_ptr<CoreConfiguration> m_currentCoreConfig;
  ScopedConnection m_emulationStartedConnection;
  void persistCoreOptions();
};

} // namespace firelight::emulation

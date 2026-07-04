#pragma once
#include "emulation_context.hpp"
#include "emulator_instance.hpp"
#include <firelight/platforms/platform.hpp>

#include <firelight/library/entry.hpp>
#include <firelight/libretro/configuration_provider.hpp>
#include <functional>
#include <memory>
#include <string>

namespace firelight::library {
class UserLibraryService;
class EntryResolver;
} // namespace firelight::library

namespace firelight::settings {
class SettingsService;
}

// Resolves core-option values for a loaded entry (global namespace).
class CoreConfiguration;

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

// Published when the active disc changes (or once on load for multi-disc
// content), so the disc UI / Discord presence / activity can react.
struct DiscChangedEvent {
  std::string contentHash;
  unsigned index;
  unsigned count;
};

// Published once on load when the core advertises selectable port devices, so
// the input UI can offer a per-port device choice. Query the current instance's
// getControllerDevices() for the details.
struct ControllerDevicesEvent {
  std::string contentHash;
};

// One-shot, per-launch knobs applied to the next loadEntry and then consumed
// (so later launches use their own defaults). These are transient launch
// parameters, distinct from the stable service dependencies in EmulationContext.
// Populated by main.cpp from the CLI; not persisted.
struct LaunchOverrides {
  int saveSlot = -1;   // >= 0 replaces the entry's stored active slot
  bool muted = false;  // start the instance muted (born muted in initialize())
};

class EmulationService {
public:
  static EmulationService *getInstance() { return s_emuServiceInstance; }
  static void setInstance(EmulationService *service) {
    s_emuServiceInstance = service;
  }

  EmulationService(library::UserLibraryService &library,
                   library::EntryResolver &entryResolver,
                   settings::SettingsService &settingsService,
                   EmulationContext context,
                   CoreFactory coreFactory = nullptr);
  ~EmulationService();

  std::future<EmulatorInstance *> loadEntry(int entryId);
  void stopEmulation();
  EmulatorInstance *getCurrentEmulatorInstance();

  // Sets the one-shot launch knobs applied to (and consumed by) the next
  // loadEntry. See LaunchOverrides. Not persisted.
  void setPendingLaunchOverrides(LaunchOverrides overrides);

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
  EmulationContext m_context;
  CoreFactory m_coreFactory;

  std::unique_ptr<EmulatorInstance> m_emulatorInstance;

  // One-shot per-launch knobs (save slot, muted, ...), applied and cleared in
  // loadEntry. Distinct from m_context's stable service dependencies.
  LaunchOverrides m_pendingLaunch;

  library::Entry m_currentEntry;
  std::string m_currentContentHash;
  platforms::Platform m_currentPlatform;
  bool m_gameRunning = false;

  // Retained so its declared options can be cached once the core has declared
  // them (on EmulationStartedEvent, i.e. after the render thread inits the core).
  std::shared_ptr<CoreConfiguration> m_currentCoreConfig;
  ScopedConnection m_emulationStartedConnection;
  void persistCoreOptions();
};

} // namespace firelight::emulation
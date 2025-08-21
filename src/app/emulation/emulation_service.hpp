#pragma once
#include "emulator_instance.hpp"
#include "platforms/models/platform.hpp"

#include <library/entry.hpp>
#include <manager_accessor.hpp>
#include <string>

namespace firelight::emulation {

struct GameLoadStarted {};
struct GameLoadedEvent {};
struct GameLoadFailedEvent {};

struct EmulationStartedEvent {};
struct EmulationStoppedEvent {};

class EmulationService : public ManagerAccessor {
public:
  static EmulationService &getInstance() {
    static EmulationService instance;
    return instance;
  }

  std::future<EmulatorInstance *> loadEntry(int entryId);
  EmulatorInstance *getCurrentEmulatorInstance();

  [[nodiscard]] bool isGameRunning() const;

  // Information about the currently running game, if any
  [[nodiscard]] std::optional<std::string> getCurrentGameName() const;
  [[nodiscard]] std::optional<library::Entry> getCurrentEntry();
  [[nodiscard]] std::optional<platforms::Platform> getCurrentPlatform() const;

private:
  std::unique_ptr<EmulatorInstance> m_emulatorInstance;

  library::Entry m_currentEntry;
  std::string m_currentContentHash;
  platforms::Platform m_currentPlatform;
  bool m_gameRunning = false;
};

} // namespace firelight::emulation
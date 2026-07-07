#pragma once

#include "firelight/libretro/audio_data_receiver.hpp"
#include "firelight/libretro/audio_input_provider.hpp"
#include "firelight/libretro/pointer_input_provider.hpp"
#include "firelight/libretro/retropad_provider.hpp"
#include "firelight/libretro/video_data_receiver.hpp"
#include "libretro/libretro.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace libretro {

class Game;

enum MemoryType {
  SAVE_RAM = RETRO_MEMORY_SAVE_RAM,
  RTC = RETRO_MEMORY_RTC,
  SYSTEM_RAM = RETRO_MEMORY_SYSTEM_RAM,
  VIDEO_RAM = RETRO_MEMORY_VIDEO_RAM
};

// Abstraction over a loaded libretro core. `Core` is the real (dlopen'd)
// implementation; tests substitute a lightweight fake. Kept to exactly the
// surface its consumers use: EmulatorInstance drives the lifecycle/state, and
// RAClient reads emulated memory for RetroAchievements.
class ICore {
public:
  virtual ~ICore() = default;

  // --- wiring (set before init/loadGame) ---
  virtual void
  setVideoReceiver(firelight::libretro::IVideoDataReceiver *receiver) = 0;
  virtual void setAudioReceiver(std::shared_ptr<IAudioDataReceiver> receiver) = 0;
  virtual void
  setRetropadProvider(firelight::libretro::IRetropadProvider *provider) = 0;
  virtual void setPointerInputProvider(
      firelight::libretro::IPointerInputProvider *provider) = 0;
  virtual void setAudioInputProvider(
      firelight::libretro::IAudioInputProvider *provider) = 0;
  virtual void setSystemDirectory(const std::string &dir) = 0;
  // Directory the core may read/write its own persistent data in (memstick,
  // pak files, NAND, …). Firelight hands it a per-game, per-slot managed path.
  virtual void setSaveDirectory(const std::string &dir) = 0;

  // --- lifecycle ---
  virtual void init() = 0;
  virtual bool loadGame(Game *game) = 0;
  virtual void run(double deltaTime) = 0;
  virtual void reset() = 0;

  // --- save data / state ---
  virtual void writeMemoryData(MemoryType memType,
                               const std::vector<char> &data) = 0;
  [[nodiscard]] virtual std::vector<char>
  getMemoryData(MemoryType memType) const = 0;
  [[nodiscard]] virtual std::vector<uint8_t> serializeState() const = 0;
  // The size of a serialized state for the loaded game (0 if the core has no
  // savestate support). Lets callers pre-size buffers and detect a state-size
  // change. Stable while a game is loaded.
  [[nodiscard]] virtual std::size_t getSerializeSize() const = 0;
  // Restores a previously serialized state. Returns false (without touching the
  // core) if the data doesn't match the current serialize size or the core
  // rejects it.
  virtual bool deserializeState(const std::vector<uint8_t> &data) const = 0;

  // --- RetroAchievements memory access ---
  [[nodiscard]] virtual void *getMemoryData(unsigned id) const = 0;
  [[nodiscard]] virtual std::size_t getMemorySize(unsigned id) const = 0;
  virtual retro_memory_map *getMemoryMap() = 0;

  // --- disc control (multi-disc games) ---
  // Number of disc images the core exposes (0 when the core has no disk-control
  // interface, i.e. single-disc/cartridge content).
  [[nodiscard]] virtual unsigned getDiskCount() const = 0;
  // Index of the currently inserted disc (0-based).
  [[nodiscard]] virtual unsigned getCurrentDiskIndex() const = 0;
  // Ejects, selects disc `index`, and re-inserts. Returns false when the core
  // has no disk control or the index is out of range.
  virtual bool setDiskIndex(unsigned index) = 0;

  // --- controller port devices ---
  // One device type a core advertises for a port (from SET_CONTROLLER_INFO),
  // e.g. {RETRO_DEVICE_JOYPAD, "N64 Controller"} or a lightgun/mouse subclass.
  struct ControllerDeviceOption {
    unsigned id;
    std::string description;
  };
  // The devices the core advertises, indexed by port. Empty (or a single entry)
  // means the port only has the default RetroPad — nothing to choose.
  [[nodiscard]] virtual std::vector<std::vector<ControllerDeviceOption>>
  getControllerDevices() const = 0;
  // Tells the core which device (RETRO_DEVICE_*) is plugged into `port`.
  virtual void setControllerPortDevice(unsigned port, unsigned device) = 0;

  // Informs the core of the resolved input device *class* on a port (the
  // firelight::input::GamepadInputClass value: 1=Joypad, 2=Mouse, 3=Light Gun)
  // so it can drive the pointer cursor from a gamepad analog stick for Mouse /
  // Light-Gun ports. Sets the per-frame glide speed (fraction of the full range
  // travelled per frame at full stick deflection). Default no-ops so test
  // doubles and pointer-less cores ignore them.
  virtual void setPortInputDeviceClass(unsigned /*port*/, int /*deviceClass*/) {}
  virtual void setAnalogPointerSpeed(double /*stepPerFrame*/) {}
  // When enabled (default), the physical mouse drives any light-gun / mouse
  // device the core presents, regardless of the port's selected device type —
  // the mouse "just works" for these games. The gamepad still only drives them
  // when that device type is selected. Default no-op.
  virtual void setMouseControlsPointerDevices(bool /*enabled*/) {}

  // --- cheats (Game Genie / Action Replay) ---
  // Applies cheat `code` (passed to the core verbatim; the core decodes the
  // format) in slot `index`, toggled by `enabled`.
  virtual void setCheat(unsigned index, bool enabled, const std::string &code) = 0;
  // Clears all applied cheats.
  virtual void clearCheats() = 0;
};

} // namespace libretro

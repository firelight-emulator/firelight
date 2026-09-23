#pragma once

#include "firelight/libretro/audio_input_provider.hpp"
#include "firelight/libretro/audio_output.hpp"
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

// Abstraction over a loaded libretro core so that we can test the emulator logic without a real core
class ICore {
public:
  virtual ~ICore() = default;

  virtual void setVideoReceiver(firelight::libretro::IVideoDataReceiver *receiver) = 0;
  virtual void setAudioReceiver(std::shared_ptr<IAudioOutput> receiver) = 0;
  virtual void setRetropadProvider(firelight::libretro::IRetropadProvider *provider) = 0;
  virtual void setPointerInputProvider(firelight::libretro::IPointerInputProvider *provider) = 0;
  virtual void setAudioInputProvider(firelight::libretro::IAudioInputProvider *provider) = 0;
  virtual void setSystemDirectory(const std::string &dir) = 0;
  virtual void setSaveDirectory(const std::string &dir) = 0;

  // lifecycle
  virtual void init() = 0;
  virtual bool loadGame(Game *game) = 0;
  virtual void run(double deltaTime) = 0;
  virtual void reset() = 0;

  // save data / state
  virtual void writeMemoryData(MemoryType memType, const std::vector<char> &data) = 0;
  [[nodiscard]] virtual std::vector<char> getMemoryData(MemoryType memType) const = 0;
  [[nodiscard]] virtual std::vector<uint8_t> serializeState() const = 0;
  [[nodiscard]] virtual std::size_t getSerializeSize() const = 0;
  virtual bool deserializeState(const std::vector<uint8_t> &data) const = 0;

  // memory
  [[nodiscard]] virtual void *getMemoryData(unsigned id) const = 0;
  [[nodiscard]] virtual std::size_t getMemorySize(unsigned id) const = 0;
  virtual retro_memory_map *getMemoryMap() = 0;

  // discs
  [[nodiscard]] virtual unsigned getDiskCount() const = 0;
  [[nodiscard]] virtual unsigned getCurrentDiskIndex() const = 0;
  virtual bool setDiskIndex(unsigned index) = 0;

  struct ControllerDeviceOption {
    unsigned id;
    std::string description;
  };

  // The devices the core advertises, indexed by port. Empty (or a single entry)
  // means the port only has the default RetroPad
  [[nodiscard]] virtual std::vector<std::vector<ControllerDeviceOption>> getControllerDevices() const = 0;
  virtual void setControllerPortDevice(unsigned port, unsigned device) = 0;

  // Informs the core of the resolved input device *class* on a port (the
  // firelight::input::GamepadInputClass value: 1=Joypad, 2=Mouse, 3=Light Gun)
  // so it can drive the pointer cursor from a gamepad analog stick for Mouse /
  // Light-Gun ports
  virtual void setPortInputDeviceClass(unsigned port, int deviceClass) {}

  virtual void setAnalogPointerSpeed(double stepPerFrame) {}

  // When enabled (default), the physical mouse drives any light-gun / mouse
  // device the core presents, regardless of the port's selected device type
  virtual void setMouseControlsPointerDevices(bool enabled) {}

  virtual void setCheat(unsigned index, bool enabled, const std::string &code) = 0;
  virtual void clearCheats() = 0;

  // Hands a key press/release to a core that asked for the keyboard. key is a
  // RETROK_* value and character is its unicode codepoint (0 if none)
  virtual void sendKeyboardEvent(bool down, unsigned key, uint32_t character, uint16_t modifiers) = 0;
  [[nodiscard]] virtual bool wantsKeyboard() const = 0;
};

} // namespace libretro

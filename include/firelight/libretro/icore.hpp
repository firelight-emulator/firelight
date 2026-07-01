#pragma once

#include "firelight/libretro/audio_data_receiver.hpp"
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
  virtual void setSystemDirectory(const std::string &dir) = 0;

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
  virtual void deserializeState(const std::vector<uint8_t> &data) const = 0;

  // --- RetroAchievements memory access ---
  [[nodiscard]] virtual void *getMemoryData(unsigned id) const = 0;
  [[nodiscard]] virtual std::size_t getMemorySize(unsigned id) const = 0;
  virtual retro_memory_map *getMemoryMap() = 0;
};

} // namespace libretro

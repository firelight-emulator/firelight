#pragma once

#include "libretro/libretro.h"

#include <cstddef>
#include <memory>
#include <string>

class QLibrary;

namespace libretro {

// Thin wrapper over a loaded libretro core DLL: owns the QLibrary + every
// resolved retro_* entry point and forwards raw calls. No policy — Core drives
// it. The constructor throws if the DLL won't load or a required symbol is
// missing.
class LibretroDll {
public:
  explicit LibretroDll(const std::string &path);
  ~LibretroDll();

  void init();
  void deinit();
  void reset();
  void run();
  bool loadGame(const retro_game_info *info);
  void unloadGame();
  void getSystemInfo(retro_system_info *info);
  void getSystemAVInfo(retro_system_av_info *info);
  void setControllerPortDevice(unsigned port, unsigned device);
  [[nodiscard]] size_t serializeSize();
  bool serialize(void *data, size_t size);
  bool unserialize(const void *data, size_t size);
  void cheatReset();
  void cheatSet(unsigned index, bool enabled, const char *code);
  [[nodiscard]] void *getMemoryData(unsigned id);
  [[nodiscard]] size_t getMemorySize(unsigned id);

  // Callback wiring, called once by Core after construction.
  void setEnvironment(retro_environment_t cb);
  void setVideoRefresh(retro_video_refresh_t cb);
  void setAudioSample(retro_audio_sample_t cb);
  void setAudioSampleBatch(retro_audio_sample_batch_t cb);
  void setInputPoll(retro_input_poll_t cb);
  void setInputState(retro_input_state_t cb);

  // Explicit (not in the destructor) so Core controls the teardown order that
  // HW-rendered cores require.
  void unload();
  [[nodiscard]] bool isLoaded() const;

private:
  std::unique_ptr<QLibrary> m_lib;

  void (*m_init)() = nullptr;
  void (*m_deinit)() = nullptr;
  unsigned (*m_apiVersion)() = nullptr;
  void (*m_getSystemInfo)(retro_system_info *) = nullptr;
  void (*m_getSystemAVInfo)(retro_system_av_info *) = nullptr;
  void (*m_setControllerPortDevice)(unsigned, unsigned) = nullptr;
  void (*m_reset)() = nullptr;
  void (*m_run)() = nullptr;
  size_t (*m_serializeSize)() = nullptr;
  bool (*m_serialize)(void *, size_t) = nullptr;
  bool (*m_unserialize)(const void *, size_t) = nullptr;
  void (*m_cheatReset)() = nullptr;
  void (*m_cheatSet)(unsigned, bool, const char *) = nullptr;
  bool (*m_loadGame)(const retro_game_info *) = nullptr;
  bool (*m_loadGameSpecial)(unsigned, const retro_game_info *, size_t) = nullptr;
  void (*m_unloadGame)() = nullptr;
  unsigned (*m_getRegion)() = nullptr;
  void *(*m_getMemoryData)(unsigned) = nullptr;
  size_t (*m_getMemorySize)(unsigned) = nullptr;

  void (*m_setEnvironment)(retro_environment_t) = nullptr;
  void (*m_setVideoRefresh)(retro_video_refresh_t) = nullptr;
  void (*m_setAudioSample)(retro_audio_sample_t) = nullptr;
  void (*m_setAudioSampleBatch)(retro_audio_sample_batch_t) = nullptr;
  void (*m_setInputPoll)(retro_input_poll_t) = nullptr;
  void (*m_setInputState)(retro_input_state_t) = nullptr;
};

} // namespace libretro

#pragma once

#include "firelight/libretro/audio_data_receiver.hpp"
#include <SDL_audio.h>

class AudioManager : public IAudioDataReceiver {
public:
  size_t receive(const int16_t *data, size_t numFrames) override;

  void initialize(double new_freq) override;

  void setMuted(bool muted);

  ~AudioManager() override;

private:
  SDL_AudioDeviceID audioDevice = 0;
  bool m_isMuted = false;
};

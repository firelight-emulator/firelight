#pragma once

#include <libretro/libretro.h>

namespace firelight::libretro {
class IAudioInputProvider {
public:
  virtual retro_microphone_t* openMicrophone() = 0;
  virtual void closeMicrophone(retro_microphone_t *microphone) = 0;
  virtual bool getMicrophoneParameters(const retro_microphone_t *microphone, retro_microphone_params_t *params) = 0;
  virtual bool setMicrophoneState(retro_microphone_t *microphone, bool state) = 0;
  virtual bool getMicrophoneState(const retro_microphone_t *microphone) = 0;
  virtual int readMicrophone(retro_microphone_t *microphone, int16_t* samples, size_t num_samples) = 0;
};
}
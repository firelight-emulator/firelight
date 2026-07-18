#pragma once

#include <firelight/libretro/audio_input_provider.hpp>

#include <memory>

namespace firelight::audio {

// TODO
// Captures the system default input device via Qt Multimedia and presents it to
// a libretro core as a microphone. One microphone, opened lazily when a core
// asks for it
class QtMicrophone final : public firelight::libretro::IAudioInputProvider {
public:
  QtMicrophone();
  ~QtMicrophone() override;

  retro_microphone_t *openMicrophone() override;
  void closeMicrophone(retro_microphone_t *microphone) override;
  bool getMicrophoneParameters(const retro_microphone_t *microphone,
                               retro_microphone_params_t *params) override;
  bool setMicrophoneState(retro_microphone_t *microphone, bool state) override;
  bool getMicrophoneState(const retro_microphone_t *microphone) override;
  int readMicrophone(retro_microphone_t *microphone, int16_t *samples,
                     size_t num_samples) override;

private:
  std::unique_ptr<retro_microphone> m_mic;
};

} // namespace firelight::audio

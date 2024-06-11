#pragma once

#include <cstdint>

class IAudioDataReceiver {
public:
  virtual size_t receive(const int16_t *data, size_t numFrames) = 0;
  virtual ~IAudioDataReceiver() = default;

  virtual void initialize(double new_freq) = 0;
  virtual bool is_muted() = 0;
  virtual void toggle_mute() = 0;
};

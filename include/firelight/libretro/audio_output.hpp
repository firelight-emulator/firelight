#pragma once

#include <cstddef>
#include <cstdint>

// Frontend audio output: receives the libretro core's audio and drives playback
// (mute, pause, buffer level, rate). Injected as a factory so the emulation code
// doesn't depend on the Qt-Multimedia impl (AudioManager); a headless/CLI app
// can supply its own
class IAudioOutput {
public:
  virtual ~IAudioOutput() = default;

  virtual size_t receive(const int16_t *data, size_t numFrames) = 0;
  virtual void initialize(double new_freq) = 0;

  virtual void setMuted(bool muted) = 0;
  [[nodiscard]] virtual bool isMuted() const = 0;
  virtual void setPaused(bool paused) = 0;
  [[nodiscard]] virtual float getBufferLevel() const = 0;
  virtual void setPlaybackRateRatio(double ratio) = 0;
  virtual void setDynamicRateControlEnabled(bool enabled) = 0;
};

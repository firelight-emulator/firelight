#pragma once

class IAudioDataReceiver {
public:
  virtual ~IAudioDataReceiver() = default;
  virtual void initialize(double new_freq) = 0;
  virtual SDL_AudioDeviceID get_audio_device() = 0;
  virtual bool is_muted() = 0;
  virtual void toggle_mute() = 0;
};

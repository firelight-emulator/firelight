//
// Created by alexs on 12/7/2023.
//

#ifndef FIRELIGHT_SDL_SOURCE_HPP
#define FIRELIGHT_SDL_SOURCE_HPP

#include "source.hpp"
#include <SDL2/SDL_audio.h>
#include <cstdio>

namespace FL::Audio {

class SDLSource : public Source {
public:
  SDLSource(int numChannels, int sampleRate, int format);
  void play() override;
  void pause() override;
  void queueSamples(int8_t *data, int n) override;
  void setSampleRate(int rate) override;
  void setAudioFormat() override;

private:
  SDL_AudioDeviceID deviceId;
};

} // namespace FL::Audio

#endif // FIRELIGHT_SDL_SOURCE_HPP

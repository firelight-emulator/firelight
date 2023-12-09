//
// Created by alexs on 12/7/2023.
//

#include "sdl_source.hpp"

namespace FL::Audio {
SDLSource::SDLSource(int numChannels, int sampleRate, int format) {
  SDL_AudioSpec want, have;

  SDL_memset(&want, 0, sizeof(want));
  want.freq = sampleRate;      // Sample rate (e.g., 44.1 kHz)
  want.format = AUDIO_S16;     // Audio format (16-bit signed)
  want.channels = numChannels; // Number of audio channels (stereo)
  want.samples = 1024;         // Audio buffer size (samples)
  want.callback = nullptr;

  deviceId = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
  if (deviceId == 0) {
    printf("SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
  }
}

void SDLSource::play() {}
void SDLSource::pause() {}
void SDLSource::queueSamples(int8_t *data, int n) {}
void SDLSource::setSampleRate(int rate) {}
void SDLSource::setAudioFormat() {}
} // namespace FL::Audio

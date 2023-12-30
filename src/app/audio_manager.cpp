//
// Created by nicho on 12/29/2023.
//

#include "audio_manager.hpp"

#include <SDL_audio.h>


void AudioManager::initialize(const double new_freq) {

    SDL_AudioSpec want, have;

    SDL_memset(&want, 0, sizeof(want));
    want.freq = new_freq;       // Sample rate (e.g., 44.1 kHz)
    want.format = AUDIO_S16; // Audio format (16-bit signed)
    want.channels = 2;       // Number of audio channels (stereo)
    want.samples = 2048;     // Audio buffer size (samples)
    want.callback = nullptr;


    this->audioDevice = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
    if (audioDevice == 0) {
        printf("SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
    }

    SDL_PauseAudioDevice(audioDevice, muted); // Start audio playback

}

SDL_AudioDeviceID AudioManager::get_audio_device() {
    return audioDevice;
}

bool AudioManager::is_muted() {
    return muted;
}

void AudioManager::toggle_mute() {
    muted = !muted;
    SDL_PauseAudioDevice(audioDevice, muted);
}


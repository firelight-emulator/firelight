//
// Created by alexs on 8/20/2024.
//

#include "SfxPlayer.hpp"

#include <spdlog/spdlog.h>


namespace firelight::audio {
    SfxPlayer::SfxPlayer() {
        SDL_AudioSpec wavSpec;
        Uint32 wavLength;

        SDL_LoadWAV("system/sfx/glass_001.wav", &wavSpec, &m_wavBuffer, &wavLength);
        m_wavLength = wavLength;

        audioDevice = SDL_OpenAudioDevice(nullptr, 0, &wavSpec, nullptr, 0);
    }

    SfxPlayer::~SfxPlayer() {
        SDL_CloseAudioDevice(audioDevice);
        SDL_FreeWAV(m_wavBuffer);
    }

    void SfxPlayer::play() {
        SDL_ClearQueuedAudio(audioDevice);
        int success = SDL_QueueAudio(audioDevice, m_wavBuffer, m_wavLength);
        if (success == -1) {
            auto message = SDL_GetError();
            printf("error: %s\n", message);
        }
        SDL_PauseAudioDevice(audioDevice, 0);
    }
} // audio
// firelight

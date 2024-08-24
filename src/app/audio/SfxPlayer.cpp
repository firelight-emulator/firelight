//
// Created by alexs on 8/20/2024.
//

#include "SfxPlayer.hpp"

#include <spdlog/spdlog.h>


namespace firelight::audio {
    SfxPlayer::SfxPlayer() {
        SDL_AudioSpec wavSpec1;
        SDL_AudioSpec wavSpec2;
        sfxStuff sfx1;
        sfxStuff sfx2;

        SDL_LoadWAV("system/sfx/click.wav", &wavSpec1, &sfx1.m_wavBuffer, &sfx1.m_wavLength);
        SDL_LoadWAV("system/sfx/openrewind.wav", &wavSpec2, &sfx2.m_wavBuffer, &sfx2.m_wavLength);

        sfx1.audioDeviceId = SDL_OpenAudioDevice(nullptr, 0, &wavSpec1, nullptr, 0);
        sfx2.audioDeviceId = SDL_OpenAudioDevice(nullptr, 0, &wavSpec2, nullptr, 0);

        m_sfxMap["rewindscroll"] = sfx1;
        m_sfxMap["rewindopen"] = sfx2;
        // m_sfxMap.insert("rewindscroll", {m_wavBuffer, m_wavLength});
    }

    SfxPlayer::~SfxPlayer() {
        for (const auto &sfx: m_sfxMap) {
            SDL_CloseAudioDevice(sfx.audioDeviceId);
            SDL_FreeWAV(sfx.m_wavBuffer);
        }
    }

    void SfxPlayer::play() {
        // SDL_ClearQueuedAudio(audioDevice);
        // int success = SDL_QueueAudio(audioDevice, m_wavBuffer, m_wavLength);
        // if (success == -1) {
        //     auto message = SDL_GetError();
        //     printf("error: %s\n", message);
        // }
        // SDL_PauseAudioDevice(audioDevice, 0);
    }

    void SfxPlayer::play(QString id) {
        auto sfx = m_sfxMap[id];
        SDL_ClearQueuedAudio(sfx.audioDeviceId);
        int success = SDL_QueueAudio(sfx.audioDeviceId, sfx.m_wavBuffer, sfx.m_wavLength);
        if (success == -1) {
            auto message = SDL_GetError();
            printf("error: %s\n", message);
        }
        SDL_PauseAudioDevice(sfx.audioDeviceId, 0);
    }
} // audio
// firelight

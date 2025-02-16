//
// Created by alexs on 8/20/2024.
//

#include "SfxPlayer.hpp"

#include <spdlog/spdlog.h>


namespace firelight::audio {
    SfxPlayer::SfxPlayer() {
        // SDL_AudioSpec wavSpec1;
        // SDL_AudioSpec wavSpec2;
        // sfxStuff sfx1;
        // sfxStuff sfx2;
        //
        // SDL_LoadWAV("system/sfx/click.wav", &wavSpec1, &sfx1.m_wavBuffer, &sfx1.m_wavLength);
        // SDL_LoadWAV("system/sfx/openrewind.wav", &wavSpec2, &sfx2.m_wavBuffer, &sfx2.m_wavLength);
        //
        // sfx1.audioDeviceId = SDL_OpenAudioDevice(nullptr, 0, &wavSpec1, nullptr, 0);
        // sfx2.audioDeviceId = SDL_OpenAudioDevice(nullptr, 0, &wavSpec2, nullptr, 0);
        //
        //
        // wavSpec1.callback = nullptr;
        // wavSpec2.callback = nullptr;
        // sfx1.wavSpec = wavSpec1;
        // sfx2.wavSpec = wavSpec2;

        m_sfxs["rewindscroll"] = new QSoundEffect();
        m_sfxs["rewindscroll"]->setSource(QUrl("qrc:sfx/click"));

        m_sfxs["rewindopen"] = new QSoundEffect();
        m_sfxs["rewindopen"]->setSource(QUrl("qrc:sfx/open-rewind"));

        m_sfxs["nope"] = new QSoundEffect();
        m_sfxs["nope"]->setSource(QUrl("qrc:sfx/nope"));

        m_sfxs["quickopen"] = new QSoundEffect();
        m_sfxs["quickopen"]->setSource(QUrl("qrc:sfx/open-quick"));

        m_sfxs["quickclose"] = new QSoundEffect();
        m_sfxs["quickclose"]->setSource(QUrl("qrc:sfx/close-quick"));

        m_sfxs["menumove"] = new QSoundEffect();
        m_sfxs["menumove"]->setSource(QUrl("qrc:sfx/menu-move"));

        m_sfxs["startgame"] = new QSoundEffect();
        m_sfxs["startgame"]->setSource(QUrl("qrc:sfx/start-game"));

        m_sfxs["defaultnotification"] = new QSoundEffect();
        m_sfxs["defaultnotification"]->setSource(QUrl("qrc:sfx/default-notification"));

        m_sfxs["switchon"] = new QSoundEffect();
        m_sfxs["switchon"]->setSource(QUrl("qrc:sfx/switch-on"));

        m_sfxs["switchoff"] = new QSoundEffect();
        m_sfxs["switchoff"]->setSource(QUrl("qrc:sfx/switch-off"));

        m_sfxs["uibump"] = new QSoundEffect();
        m_sfxs["uibump"]->setSource(QUrl("qrc:sfx/ui-bump"));

        // m_sfxs["rewindscroll"] = std::move(effect1);
        // m_sfxs["rewindopen"] = std::move(effect2);

        // m_sfxMap["rewindscroll"] = sfx1;
        // m_sfxMap["rewindopen"] = sfx2;
        // m_sfxMap.insert("rewindscroll", {m_wavBuffer, m_wavLength});
    }

    SfxPlayer::~SfxPlayer() {
        // for (const auto &sfx: m_sfxMap) {
        //     SDL_CloseAudioDevice(sfx.audioDeviceId);
        //     SDL_FreeWAV(sfx.m_wavBuffer);
        // }
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

    void SfxPlayer::play(const QString &id) {
        if (m_sfxs.contains(id)) {
            // m_sfxs[id]->stop();
            m_sfxs[id]->play();
        }

        // auto sfx = m_sfxMap[id];
        // SDL_ClearQueuedAudio(sfx.audioDeviceId);
        // int success = SDL_QueueAudio(sfx.audioDeviceId, sfx.m_wavBuffer, sfx.m_wavLength);
        // if (success == -1) {
        //     auto message = SDL_GetError();
        //     printf("error: %s\n", message);
        // }
        // SDL_PauseAudioDevice(sfx.audioDeviceId, 0);
    }
} // audio
// firelight

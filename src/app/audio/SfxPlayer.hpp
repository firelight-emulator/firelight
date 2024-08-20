#pragma once
#include <QObject>
#include <SDL_audio.h>


namespace firelight::audio {
    class SfxPlayer : public QObject {
        Q_OBJECT

    public:
        SfxPlayer();

        ~SfxPlayer();

        Q_INVOKABLE void play();

        // SDL_AudioSpec want, have;
        //
        // SDL_memset(&want, 0, sizeof(want));
        // want.freq = new_freq; // Sample rate (e.g., 44.1 kHz)
        // want.format = AUDIO_S16; // Audio format (16-bit signed)
        // want.channels = 2; // Number of audio channels (stereo)
        // want.samples = 128; // Audio buffer size (samples)
        // want.callback = nullptr;
        //
        // this->audioDevice = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
        // if (audioDevice == 0) {
        //     printf("SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
        // }
        //
        // SDL_PauseAudioDevice(audioDevice, muted); // Start audio playback
    private:
        SDL_AudioDeviceID audioDevice = 0;
        Uint8 *m_wavBuffer{};
        Uint32 m_wavLength;
    };
} // audio
// firelight

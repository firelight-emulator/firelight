//
// Created by nicho on 12/29/2023.
//

#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H
#include <SDL_audio.h>

#include "libretro/audio_data_receiver.hpp"


class AudioManager: public CoreAudioDataReceiver {
    SDL_AudioDeviceID audioDevice = 0;
public:
    void initialize(double new_freq) override;
    SDL_AudioDeviceID getAudioDevice() override;
};



#endif //AUDIO_MANAGER_H

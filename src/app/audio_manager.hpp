//
// Created by nicho on 12/29/2023.
//

#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H
#include <SDL_audio.h>

#include "libretro/audio_data_receiver.hpp"


class AudioManager: public IAudioDataReceiver {
    SDL_AudioDeviceID audioDevice = 0;
public:
    void initialize(double new_freq) override;
    SDL_AudioDeviceID get_audio_device() override;
    bool is_muted() override;
    void toggle_mute() override;
private:
    bool muted;
};



#endif //AUDIO_MANAGER_H

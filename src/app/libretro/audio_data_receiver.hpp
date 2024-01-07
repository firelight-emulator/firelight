//
// Created by nicho on 12/29/2023.
//

#ifndef AUDIO_DATA_RECEIVER_H
#define AUDIO_DATA_RECEIVER_H

class CoreAudioDataReceiver {

public:
    virtual ~CoreAudioDataReceiver() = default;
    virtual void initialize(double new_freq) = 0;
    virtual SDL_AudioDeviceID get_audio_device() = 0;
    virtual bool is_muted() = 0;
    virtual void toggle_mute() = 0;
};

#endif //AUDIO_DATA_RECEIVER_H

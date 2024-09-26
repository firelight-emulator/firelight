#pragma once

#include <array>
#include <QBuffer>
#include <QAudioSink>
#include <queue>

extern "C" {
#include <libswresample/swresample.h>
#include <libavutil/samplefmt.h>
}


#include "firelight/libretro/audio_data_receiver.hpp"
#include <SDL_audio.h>
#include <vector>

class AudioManager : public IAudioDataReceiver {
public:
    size_t receive(const int16_t *data, size_t numFrames) override;

    void initialize(double new_freq) override;

    void setMuted(bool muted);

    ~AudioManager() override;

    QBuffer m_buffer{};

private:
    SwrContext *m_swrContext = nullptr;
    QAudioSink *m_audioSink = nullptr;
    QIODevice *m_audioDevice = nullptr;
    std::array<int16_t, 4096> m_audioBuffer{};
    int m_audioBufferIndex = 0;
    size_t m_bufferSize = 0;
    std::queue<int16_t> m_audioBuffer2;
    SDL_AudioDeviceID audioDevice = 0;
    bool m_isMuted = false;

    int m_sampleRate = 0;

    void initializeResampler(int64_t in_channel_layout, int in_sample_rate, enum AVSampleFormat in_sample_fmt,
                             int64_t out_channel_layout, int out_sample_rate, enum AVSampleFormat out_sample_fmt);

    int resampleAudio(uint8_t **in_data, int in_nb_samples, uint8_t **out_data, int out_nb_samples, int delta_samples);
};

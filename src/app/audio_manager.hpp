#pragma once

#include <array>
#include <QBuffer>
#include <QAudioSink>
#include <qelapsedtimer.h>
#include <queue>

extern "C" {
#include <libswresample/swresample.h>
#include <libavutil/samplefmt.h>
#include <libavutil/opt.h>
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

private:
    int m_frameNumber = 0;
    SwrContext *m_swrContext = nullptr;
    QAudioSink *m_audioSink = nullptr;
    QIODevice *m_audioDevice = nullptr;

    double m_changeThing = 0.0;
    uint64_t m_deltaFrames = 0;

    size_t m_bufferSize = 0;
    bool m_isMuted = false;
    size_t m_numSamples = 0;

    uint64_t m_avgNumFrames = 0;

    int m_sampleRate = 0;

    void initializeResampler(int64_t in_channel_layout, int in_sample_rate, enum AVSampleFormat in_sample_fmt,
                             int64_t out_channel_layout, int out_sample_rate, enum AVSampleFormat out_sample_fmt);
};

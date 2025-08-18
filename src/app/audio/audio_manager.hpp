#pragma once

#include <QAudioSink>
#include <QBuffer>
#include <array>
#include <qelapsedtimer.h>
#include <queue>
#include <shared_mutex>

extern "C" {
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
}

#include "firelight/libretro/audio_data_receiver.hpp"
#include <SDL_audio.h>
#include <vector>

class AudioManager : public QIODevice, public IAudioDataReceiver {
public:
  explicit AudioManager(
      std::function<void()> onAudioBufferLevelChanged = nullptr);

  size_t receive(const int16_t *data, size_t numFrames) override;

  void initialize(double new_freq) override;

  void setMuted(bool muted);

  bool isMuted() const;

  float getBufferLevel() const;

  ~AudioManager() override;

  qint64 size() const override;
  qint64 bytesAvailable() const override;

  qint64 readData(char *data, qint64 maxlen) override;
  qint64 writeData(const char *data, qint64 len) override;

private:
  std::shared_mutex m_mutex;
  std::array<char, 8192 * 2> m_audioBuffer;
  size_t m_bufferWritePos = 0;
  size_t m_bufferReadPos = 0;

  std::function<void()> m_onAudioBufferLevelChanged;

  float m_currentBufferLevel = 0.0f;

  QElapsedTimer m_elapsedTimer;
  long m_numSamples = 0;

  int m_frameNumber = 0;
  SwrContext *m_swrContext = nullptr;
  QAudioSink *m_audioSink = nullptr;

  double m_changeThing = 0.0;
  uint64_t m_deltaFrames = 0;

  size_t m_bufferSize = 0;
  bool m_isMuted = false;

  uint64_t m_avgNumFrames = 0;

  int m_sampleRate = 0;

  AVChannelLayout *m_channelLayout = new AVChannelLayout;

  void initializeResampler(int64_t in_channel_layout, int in_sample_rate,
                           enum AVSampleFormat in_sample_fmt,
                           int64_t out_channel_layout, int out_sample_rate,
                           enum AVSampleFormat out_sample_fmt);
};

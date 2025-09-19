#pragma once

#include <QAudioSink>
#include <QBuffer>
#include <QMediaDevices>
#include <array>
#include <qelapsedtimer.h>
#include <queue>

extern "C" {
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
}

#include "firelight/libretro/audio_data_receiver.hpp"
#include <SDL_audio.h>
#include <vector>

class AudioManager : public QObject, public IAudioDataReceiver {
  Q_OBJECT
public:
  explicit AudioManager(
      std::function<void()> onAudioBufferLevelChanged = nullptr);

  size_t receive(const int16_t *data, size_t numFrames) override;

  void initialize(double new_freq) override;

  void setMuted(bool muted);

  bool isMuted() const;

  float getBufferLevel() const;

  ~AudioManager() override;

private:
  std::function<void()> m_onAudioBufferLevelChanged;

  float m_currentBufferLevel = 0.0f;

  QElapsedTimer m_elapsedTimer;
  long m_numSamples = 0;

  int m_frameNumber = 0;
  SwrContext *m_swrContext = nullptr;
  QAudioSink *m_audioSink = nullptr;
  QIODevice *m_audioDevice = nullptr;

  double m_changeThing = 0.0;
  uint64_t m_deltaFrames = 0;

  size_t m_bufferSize = 0;
  bool m_isMuted = false;

  uint64_t m_avgNumFrames = 0;

  int m_sampleRate = 0;

  AVChannelLayout *m_channelLayout = new AVChannelLayout;

  QMediaDevices *m_mediaDevices = nullptr;

  // Moving average state
  static constexpr int AVG_WINDOW_SIZE = 10;
  int m_avgBufferUsageBytes[AVG_WINDOW_SIZE] = {};
  int m_avgBufferIdx = 0;
  int m_avgBufferPopulatedCount = 0;
  double m_previousAvgFillRatio = -1.0;

  void initializeResampler(int64_t in_channel_layout, int in_sample_rate,
                           enum AVSampleFormat in_sample_fmt,
                           int64_t out_channel_layout, int out_sample_rate,
                           enum AVSampleFormat out_sample_fmt);

  void reinitializeAudioDevice();

private slots:
  void onAudioDevicesChanged();
};

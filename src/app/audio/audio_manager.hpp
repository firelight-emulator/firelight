#pragma once

#include <QAudioSink>
#include <QMediaDevices>
#include <atomic>
#include <functional>
#include <memory>

#include "audio_rate_controller.hpp"
#include "audio_resampler.hpp"
#include "firelight/libretro/audio_data_receiver.hpp"

class AudioManager : public QObject, public IAudioDataReceiver {
  Q_OBJECT
public:
  explicit AudioManager(
      std::function<void()> onAudioBufferLevelChanged = nullptr);

  size_t receive(const int16_t *data, size_t numFrames) override;

  void initialize(double new_freq) override;

  void setMuted(bool muted);

  bool isMuted() const;

  // Suspends/resumes the output device. On pause we stop feeding the sink, so
  // without this the device keeps draining its queued buffer (an audible "tail");
  // suspend() halts playback immediately and preserves the buffer for a seamless
  // resume.
  void setPaused(bool paused);

  // Buffer fill ratio (0.0-1.0). Safe to read from other threads (e.g. the
  // emulation pacing thread when sync method is "audio").
  float getBufferLevel() const;

  // Biases the resampler so audio plays back `ratio`x faster/slower than the
  // core's native rate (1.0 = native). Used by sync-to-monitor to resample audio
  // to the display's refresh rate (ratio = refreshHz / coreFps) so it stays
  // matched to the paced video. Dynamic rate control still corrects residual drift.
  void setPlaybackRateRatio(double ratio);

  ~AudioManager() override;

private:
  std::function<void()> m_onAudioBufferLevelChanged;

  std::atomic<float> m_currentBufferLevel = 0.0f;

  // Converts core audio to the device rate; owns the feed-forward playback-rate
  // bias and applies the drift-compensation delta.
  AudioResampler m_resampler;
  // Decides that delta from the output buffer's occupancy (dynamic rate control).
  AudioRateController m_rateController;

  std::unique_ptr<QAudioSink> m_audioSink;
  QIODevice *m_audioDevice = nullptr;

  bool m_isMuted = false;
  int m_sampleRate = 0;

  QMediaDevices *m_mediaDevices = nullptr;

  // Creates m_audioSink for the current device + sample rate and starts it.
  void openAudioSink();

  void reinitializeAudioDevice();

private slots:
  void onAudioDevicesChanged();
};

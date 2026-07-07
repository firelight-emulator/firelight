#pragma once

#include <QAudioSink>
#include <QMediaDevices>
#include <atomic>
#include <functional>
#include <memory>

#include <firelight/audio/audio_rate_controller.hpp>
#include <firelight/audio/audio_resampler.hpp>
#include "firelight/libretro/audio_output.hpp"

// Threading: created on the render thread (injected into EmulatorInstance),
// where receive() is called each frame. The QAudioSink runs on Qt's audio
// thread. Buffer-level/rate/mute reads come from the pacing thread, so the
// fields those touch are atomic.
class AudioManager : public QObject, public IAudioOutput {
  Q_OBJECT
public:
  explicit AudioManager(
      std::function<void()> onAudioBufferLevelChanged = nullptr);

  size_t receive(const int16_t *data, size_t numFrames) override;

  void initialize(double new_freq) override;

  void setMuted(bool muted) override;

  bool isMuted() const override;

  // Suspends/resumes the output device. On pause we stop feeding the sink, so
  // without this the device keeps draining its queued buffer (an audible "tail");
  // suspend() halts playback immediately and preserves the buffer for a seamless
  // resume.
  void setPaused(bool paused) override;

  // Buffer fill ratio (0.0-1.0). Safe to read from other threads (e.g. the
  // emulation pacing thread when sync method is "audio").
  float getBufferLevel() const override;

  // Biases the resampler so audio plays back `ratio`x faster/slower than the
  // core's native rate (1.0 = native). Used by sync-to-monitor to resample audio
  // to the display's refresh rate (ratio = refreshHz / coreFps) so it stays
  // matched to the paced video. Dynamic rate control still corrects residual drift.
  void setPlaybackRateRatio(double ratio) override;

  // Enables/disables Dynamic Rate Control: the drift compensation that nudges the
  // resample rate to keep the sink buffer near 50% full. On by default; exposed
  // as an advanced setting so users can let the emulation pacer manage the buffer
  // alone. Thread-safe (read from the audio thread).
  void setDynamicRateControlEnabled(bool enabled) override;

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
  int m_sampleRate = 0;       // the core's audio rate
  int m_deviceSampleRate = 0; // the output device's native rate (sink runs here)

  // Dynamic Rate Control on by default; toggled by the "dynamic-rate-control"
  // advanced setting.
  std::atomic<bool> m_drcEnabled{true};

  // Pre-buffering. The sink starts suspended so it doesn't drain an empty buffer
  // (an underrunning, stuttery "skip-ahead" start); receive() fills it and
  // resumes playback once it's ~half full.
  std::atomic<bool> m_priming = true;

  QMediaDevices *m_mediaDevices = nullptr;

  // Creates m_audioSink for the current device + sample rate and starts it.
  void openAudioSink();

  void reinitializeAudioDevice();

private slots:
  void onAudioDevicesChanged();
};

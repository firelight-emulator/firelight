#pragma once

#include "audio_settings.hpp"
#include "firelight/libretro/audio_output.hpp"

#include <firelight/audio/audio_rate_controller.hpp>
#include <firelight/audio/audio_resampler.hpp>
#include <firelight/event_dispatcher.hpp>
#include <firelight/settings/settings_service.hpp>

#include <QAudioSink>
#include <QMediaDevices>
#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>

// Threading: created on the render thread (injected into EmulatorInstance),
// where receive() is called each frame. The QAudioSink runs on Qt's audio
// thread. Buffer-level/rate/mute reads come from the pacing thread, so the
// fields those touch are atomic
class AudioManager : public QObject, public IAudioOutput {
  Q_OBJECT
public:
  // Both live in audio_settings.hpp so a caller can name a key without pulling
  // Qt Multimedia in through this header. Read here rather than pushed in, which
  // is what makes mute outlive a game: this class is rebuilt on every load, so
  // it picks up the current value each time instead of starting unmuted
  static constexpr auto OUTPUT_DEVICE_KEY = firelight::audio::OUTPUT_DEVICE_KEY;
  static constexpr auto MUTED_KEY = firelight::audio::MUTED_KEY;
  static constexpr auto VOLUME_KEY = firelight::audio::VOLUME_KEY;
  static constexpr auto LATENCY_KEY = firelight::audio::LATENCY_KEY;

  explicit AudioManager(firelight::settings::SettingsService &settingsService,
                        std::function<void()> onAudioBufferLevelChanged = nullptr);

  size_t receive(const int16_t *data, size_t numFrames) override;

  void initialize(double new_freq) override;

  // Transient silencing driven by the running emulator (pause, fast-forward) —
  // not the user's mute, which this class reads from MUTED_KEY itself. Output is
  // silent when either is set
  void setMuted(bool muted) override;

  bool isMuted() const override;

  // Suspends/resumes the output device. On pause we stop feeding the sink, so
  // without this the device keeps draining its queued buffer (an audible "tail");
  // suspend() halts playback immediately and preserves the buffer for a seamless
  // resume
  void setPaused(bool paused) override;

  // Buffer fill ratio (0.0-1.0). Safe to read from other threads (e.g. the
  // emulation pacing thread when sync method is "audio")
  float getBufferLevel() const override;

  // Biases the resampler so audio plays back `ratio`x faster/slower than the
  // core's native rate (1.0 = native). Used by sync-to-monitor to resample audio
  // to the display's refresh rate (ratio = refreshHz / coreFps) so it stays
  // matched to the paced video. Dynamic rate control still corrects residual drift
  void setPlaybackRateRatio(double ratio) override;

  // Enables/disables Dynamic Rate Control: the drift compensation that nudges the
  // resample rate to keep the sink buffer near 50% full. On by default; exposed
  // as an advanced setting so users can let the emulation pacer manage the buffer
  // alone. Thread-safe (read from the audio thread)
  void setDynamicRateControlEnabled(bool enabled) override;

  ~AudioManager() override;

private:
  // The output the user picked, or the system default when unset or gone
  [[nodiscard]] QAudioDevice selectedOutputDevice() const;

  // Re-reads MUTED_KEY into m_userMuted
  // TODO
  /** How much sound the user wants buffered ahead, clamped to what the sink can usefully do */
  [[nodiscard]] int latencyMs() const;

  void refreshUserMuted();
  // Re-reads VOLUME_KEY and applies it to the sink, if there is one
  void refreshVolume();

  firelight::settings::SettingsService &m_settingsService;
  ScopedConnection m_settingChangedConnection;
  ScopedConnection m_settingResetConnection;
  std::function<void()> m_onAudioBufferLevelChanged;

  std::atomic<float> m_currentBufferLevel = 0.0f;

  // Converts core audio to the device rate; owns the feed-forward playback-rate
  // bias and applies the drift-compensation delta
  AudioResampler m_resampler;
  // Decides that delta from the output buffer's occupancy (dynamic rate control)
  AudioRateController m_rateController;

  std::unique_ptr<QAudioSink> m_audioSink;
  QIODevice *m_audioDevice = nullptr;

  // QAudioSink/QIODevice aren't thread-safe. receive() (render thread) and
  // getBufferLevel() (the "audio" sync method's pacing thread) both touch the
  // sink, so all access is serialized on this. Held only for the brief sink
  // calls — never across a core frame
  mutable std::mutex m_sinkMutex;

  // Transient, set by the emulator (pause / fast-forward). Written on the GUI
  // thread, read by receive() on the render thread
  std::atomic<bool> m_isMuted{false};
  // The user's MUTED_KEY setting, refreshed when it changes
  std::atomic<bool> m_userMuted{false};
  // 0-1. Held so a sink created later (device change) starts at the right
  // loudness rather than full
  std::atomic<float> m_volume{1.0f};

  int m_sampleRate = 0;       // the core's audio rate
  int m_deviceSampleRate = 0; // the output device's native rate (sink runs here)

  // Dynamic Rate Control on by default; toggled by the "dynamic-rate-control"
  // advanced setting
  std::atomic<bool> m_drcEnabled{true};

  // Pre-buffering. The sink starts suspended so it doesn't drain an empty buffer
  // (an underrunning, stuttery "skip-ahead" start); receive() fills it and
  // resumes playback once it's ~half full
  std::atomic<bool> m_priming = true;

  QMediaDevices *m_mediaDevices = nullptr;

  // When the next rebuild may be attempted, so an output that is simply gone is not reopened on
  // every frame, and how long the wait after that is
  std::chrono::steady_clock::time_point m_nextReopenAttempt{};
  int m_reopenBackoffMs = 0;

  // Creates m_audioSink for the current device + sample rate and starts it
  void openAudioSink();

  // Rebuilds the sink when the output it was running on has stopped it. Callers hold m_sinkMutex
  void recoverFromDeviceLoss();

  void reinitializeAudioDevice();

  // Whether the sink can still be written to. A stopped sink has already invalidated the QIODevice
  // that start() handed back. Callers hold m_sinkMutex
  [[nodiscard]] bool isSinkWritable() const;

  // Whether the sink's occupancy can be read at all: bytesFree() answers zero outside these two
  // states, which reads as a completely full buffer. Callers hold m_sinkMutex
  [[nodiscard]] bool isSinkMeasurable() const;

private slots:
  void onAudioDevicesChanged();
};

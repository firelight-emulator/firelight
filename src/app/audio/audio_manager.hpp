// TODO: NEEDS REVIEW
#pragma once

#include "audio_settings.hpp"
#include "firelight/libretro/audio_output.hpp"

#include <firelight/audio/audio_rate_controller.hpp>
#include <firelight/audio/audio_resampler.hpp>
#include <firelight/audio/playback_buffer.hpp>
#include <firelight/event_dispatcher.hpp>
#include <firelight/monitoring/monitor.hpp>
#include <firelight/settings/settings_service.hpp>

#include <atomic>
#include <functional>
#include <memory>
#include <string>

namespace firelight::audio {
class AudioOutputStream;
}

/**
 * Takes the core's sound, resamples it to the output's rate with drift correction, and pushes it
 * into the buffer the output stream plays from.
 *
 * Threading: receive() and initialize() run on whichever thread runs frames; the setters and
 * getBufferLevel() may be called from any thread. Nothing here touches Qt Multimedia, which lives
 * in the AudioOutputStream on the GUI thread
 */
class AudioManager : public IAudioOutput {
public:
  // Both live in audio_settings.hpp so a caller can name a key without pulling
  // Qt Multimedia in through this header. Read here rather than pushed in, which
  // is what makes mute outlive a game: this class is rebuilt on every load, so
  // it picks up the current value each time instead of starting unmuted
  static constexpr auto OUTPUT_DEVICE_KEY = firelight::audio::OUTPUT_DEVICE_KEY;
  static constexpr auto MUTED_KEY = firelight::audio::MUTED_KEY;
  static constexpr auto VOLUME_KEY = firelight::audio::VOLUME_KEY;
  static constexpr auto LATENCY_KEY = firelight::audio::LATENCY_KEY;

  /**
   * @param contentHash The game being buffered for, so a per-game latency override resolves. Empty
   *   where there is no local game, as when playing back a guest's stream
   */
  explicit AudioManager(firelight::settings::SettingsService &settingsService, std::string contentHash = {},
                        int platformId = 0, std::function<void()> onAudioBufferLevelChanged = nullptr);

  ~AudioManager() override;

  size_t receive(const int16_t *data, size_t numFrames) override;

  void initialize(double new_freq) override;

  // Transient silencing driven by the running emulator (pause, fast-forward) —
  // not the user's mute, which this class reads from MUTED_KEY itself. Output is
  // silent when either is set
  void setMuted(bool muted) override;

  bool isMuted() const override;

  // TODO
  /**
   * Holds playback where it is; what is buffered plays on when unpaused
   */
  void setPaused(bool paused) override;

  // TODO
  /**
   * How full the playback buffer is, 0 to 1, or -1 while nothing is playing
   */
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

private:
  // Re-reads MUTED_KEY into m_userMuted
  void refreshUserMuted();

  firelight::settings::SettingsService &m_settingsService;
  ScopedConnection m_settingChangedConnection;
  ScopedConnection m_settingResetConnection;
  std::function<void()> m_onAudioBufferLevelChanged;

  std::shared_ptr<firelight::audio::PlaybackBuffer> m_buffer;

  // TODO
  // On the GUI thread, so deleted through the event loop rather than here
  firelight::audio::AudioOutputStream *m_stream = nullptr;

  // Converts core audio to the device rate; owns the feed-forward playback-rate
  // bias and applies the drift-compensation delta
  AudioResampler m_resampler;
  // Decides that delta from the output buffer's occupancy (dynamic rate control)
  AudioRateController m_rateController;

  // Transient, set by the emulator (pause / fast-forward). Written on the GUI
  // thread, read by receive() on the frame thread
  std::atomic<bool> m_isMuted{false};
  // The user's MUTED_KEY setting, refreshed when it changes
  std::atomic<bool> m_userMuted{false};

  int m_sampleRate = 0;       // the core's audio rate
  int m_deviceSampleRate = 0; // the rate the resampler was last built for

  // Dynamic Rate Control on by default; toggled by the "dynamic-rate-control"
  // advanced setting
  std::atomic<bool> m_drcEnabled{true};

  firelight::monitoring::Span m_audioBatchSpan = firelight::monitoring::Monitor::instance().span(
      "audio_batch", "Taking one batch of samples from the core through to the playback buffer");
  firelight::monitoring::Span m_resampleSpan =
      firelight::monitoring::Monitor::instance().span("resample", "Resampling one batch to the output's rate");
  firelight::monitoring::Series m_audioBufferSeries =
      firelight::monitoring::Monitor::instance().series("audio_buffer", "How full the playback buffer is, 0 to 1");
  firelight::monitoring::Series m_audioCorrectionSeries = firelight::monitoring::Monitor::instance().series(
      "audio_correction", "The rate correction applied to keep the playback buffer half full");
  firelight::monitoring::Series m_audioSamplesSeries = firelight::monitoring::Monitor::instance().series(
      "audio_samples", "How many sample frames one batch put into the playback buffer");
};

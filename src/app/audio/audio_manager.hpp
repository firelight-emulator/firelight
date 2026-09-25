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
 * into the buffer the output stream plays from
 */
class AudioManager final : public IAudioOutput {
public:
  static constexpr auto MUTED_KEY = firelight::audio::MUTED_KEY;

  /**
   * @param contentHash The game being buffered for, so a per-game latency override resolves. Empty
   *   where there is no local game, as when playing back a guest's stream
   */
  explicit AudioManager(firelight::settings::SettingsService &settingsService, std::string contentHash = {},
                        int platformId = 0, std::function<void()> onAudioBufferLevelChanged = nullptr);

  ~AudioManager() override;

  size_t receive(const int16_t *data, size_t numFrames) override;

  void initialize(double newFreq) override;

  /**
   * For programmatic muting like when the emulator is paused or fast-forwarding, not the user's mute setting
   */
  void setMuted(bool muted) override;

  [[nodiscard]] bool isMuted() const override;

  /**
   * Holds playback where it is, what is buffered plays on when unpaused
   */
  void setPaused(bool paused) override;

  /**
   * How full the playback buffer is, 0 to 1, or -1 while nothing is playing
   */
  [[nodiscard]] float getBufferLevel() const override;

  /**
   * The cores expect to be able to produce sound at their native rate, so this lets you correct for running it faster
   * or slower than that.
   */
  void setPlaybackRateRatio(double ratio) override;

  void setDynamicRateControlEnabled(bool enabled) override;

private:
  // Re-reads MUTED_KEY into m_userMuted
  void refreshUserMuted();

  firelight::settings::SettingsService &m_settingsService;
  ScopedConnection m_settingChangedConnection;
  ScopedConnection m_settingResetConnection;
  std::function<void()> m_onAudioBufferLevelChanged;

  std::shared_ptr<firelight::audio::PlaybackBuffer> m_buffer;

  // On the GUI thread, so deleted through the event loop rather than here
  firelight::audio::AudioOutputStream *m_stream = nullptr;

  // Converts core audio to the device rate and applies DRC
  AudioResampler m_resampler;

  // Decides the amount to correct the resampler's rate for DRC, and smooths the correction over time to avoid audible
  // artifacts
  AudioRateController m_rateController;

  // Set by the emulator, not the user
  std::atomic<bool> m_isMuted{false};

  // Set by the user's setting, not the emulator
  std::atomic<bool> m_userMuted{false};

  int m_sampleRate = 0;       // the core's audio rate
  int m_deviceSampleRate = 0; // the rate the resampler was last built for

  // Dynamic Rate Control, on by default
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

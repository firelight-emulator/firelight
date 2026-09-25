#pragma once

#include <firelight/audio/playback_buffer.hpp>
#include <firelight/event_dispatcher.hpp>
#include <firelight/settings/settings_service.hpp>

#include <QAudioSink>
#include <QMediaDevices>
#include <QObject>
#include <QTimer>
#include <atomic>
#include <memory>
#include <string>

namespace firelight::audio {

/**
 * One stereo output stream, played from a PlaybackBuffer by Qt's audio thread. Follows the output
 * device, latency and volume settings, and reopens when the output it runs on changes or fails
 */
class AudioOutputStream : public QObject {
  Q_OBJECT

public:
  /**
   * @param contentHash The game the latency setting resolves against; empty when there is none
   */
  AudioOutputStream(settings::SettingsService &settingsService, std::string contentHash, int platformId,
                    std::shared_ptr<PlaybackBuffer> buffer);

  ~AudioOutputStream() override;

  /**
   * Opens a stream for sound produced at coreSampleRate, replacing any stream already open
   */
  void open(int coreSampleRate);

  /**
   * @return The rate the stream plays at, or 0 while there is no stream
   */
  [[nodiscard]] int getSampleRate() const;

private slots:
  /**
   * Reopens the stream when the machine's set of audio outputs changes
   */
  void onAudioOutputsChanged();

  /**
   * Reopens the stream after the output it was running on stopped it with an error
   */
  void onSinkStateChanged(QtAudio::State state);

private:
  /**
   * Tears the stream down and starts a new one on the selected output
   */
  void reopen();

  /**
   * Reopens after delayMs, replacing any reopen already waiting
   */
  void scheduleReopen(int delayMs);

  /**
   * Re-reads the volume setting and applies it to the stream
   */
  void refreshVolume();

  /**
   * How much sound the user wants buffered ahead
   */
  [[nodiscard]] int latencyMs() const;

  settings::SettingsService &m_settingsService;
  std::string m_contentHash;
  int m_platformId;
  std::shared_ptr<PlaybackBuffer> m_buffer;

  QMediaDevices *m_mediaDevices = nullptr;
  QTimer *m_reopenTimer = nullptr;

  // Grows each time a reopen fails so an output that is simply gone is not retried on a loop, and resets as soon as
  // a stream starts
  int m_reopenBackoffMs = 0;

  ScopedConnection m_settingChangedConnection;
  ScopedConnection m_settingResetConnection;

  // 0-1, set by the user's master-volume setting
  float m_volume = 1.0F;

  int m_coreSampleRate = 0;
  std::atomic<int> m_sampleRate{0};

  std::unique_ptr<QAudioSink> m_sink;
};

} // namespace firelight::audio

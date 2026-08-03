#pragma once

#include <firelight/audio/sfx_mixer.hpp>
#include <firelight/event_dispatcher.hpp>
#include <firelight/settings/settings_service.hpp>

#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioSink>
#include <QByteArray>
#include <QHash>
#include <QMediaDevices>
#include <QObject>
#include <QSpan>
#include <QString>
#include <QTimer>
#include <QUrl>
#include <atomic>
#include <memory>
#include <vector>

namespace firelight::audio {

/**
 * Plays short UI sounds through one persistent output stream.
 *
 * The stream is opened once and kept rendering for the life of the object, feeding silence
 * whenever nothing is sounding. That is deliberate: Windows powers the output path down after a
 * few hundred milliseconds of silence and eats the head of whatever plays next, which is long
 * enough to swallow the shortest UI clips entirely.
 *
 * Threading: constructed and called from the GUI thread. The mixer's render callback runs on Qt's
 * audio thread, and play()/stop() reach it through a lock-free ring, so no call here blocks.
 */
class UiSoundPlayer : public QObject {
  Q_OBJECT

public:
  explicit UiSoundPlayer(settings::SettingsService &settingsService, QObject *parent = nullptr);

  ~UiSoundPlayer() override;

  /**
   * Reads, decodes, and converts a sound to the output's format, or returns the id it already has
   *
   * @param source A qrc: or file: URL naming a RIFF/WAVE file
   * @return The clip's id, or -1 if it could not be loaded
   */
  Q_INVOKABLE int registerClip(const QUrl &source);

  /**
   * Starts a clip on a new voice
   *
   * @param clipId An id from registerClip
   * @param gain Linear gain, already scaled by the caller's own volume
   * @param voices How many voices of this clip may sound at once
   * @param pitch Playback rate multiplier; 1.0 plays the clip at its recorded pitch
   */
  Q_INVOKABLE void play(int clipId, qreal gain, int voices, qreal pitch = 1.0);

  /**
   * Fades out every voice currently playing a clip
   */
  Q_INVOKABLE void stop(int clipId);

  /**
   * @return Whether the output stream is open, so sounds will actually be heard
   */
  [[nodiscard]] Q_INVOKABLE bool isRunning() const;

private slots:
  /**
   * Reopens the stream when the machine's set of audio outputs changes
   */
  void onAudioOutputsChanged();

private:
  /**
   * Picks a format the device actually supports, preferring its own preferred format
   */
  [[nodiscard]] static QAudioFormat chooseFormat(const QAudioDevice &device);

  /**
   * Tears the stream down, reconverts every clip when the format changed, and starts a new stream
   */
  void reopen();

  /**
   * Reconverts every retained source file to the mixer's current rate and channel count
   */
  void rebuildClips();

  /**
   * Re-reads the interface volume setting and applies it to the stream
   */
  void refreshVolume();

  /**
   * Decodes one file's bytes and hands the samples to the mixer
   *
   * @return The clip's id, or -1 if the bytes could not be decoded
   */
  int addClipFromBytes(const QByteArray &bytes);

  /**
   * Starts the sink with a callback matching the chosen sample type
   */
  template <typename SampleType> void startSink(const QAudioDevice &device);

  /**
   * Fills one callback buffer from the mixer, converting out of float when the device needs it
   */
  template <typename SampleType> void renderInto(QSpan<SampleType> buffer);

  /**
   * Mixes in the inaudible noise floor that stops the output hardware powering down
   */
  void addKeepAlive(float *samples, size_t frames, size_t channels);

  /**
   * Turns a qrc: or file: URL into a path QFile can open
   */
  [[nodiscard]] static QString resolveResourcePath(const QUrl &source);

  settings::SettingsService &m_settingsService;
  SfxMixer m_mixer;

  // The raw bytes of every registered file, kept so clips can be reconverted when the output
  // format changes
  std::vector<QByteArray> m_clipSources;
  QHash<QString, int> m_clipIdsByPath;

  QMediaDevices *m_mediaDevices = nullptr;
  QByteArray m_deviceId;
  QAudioFormat m_format;
  QTimer m_reopenTimer;

  ScopedConnection m_settingChangedConnection;
  ScopedConnection m_settingResetConnection;

  // 0-1, perceptually mapped from the interface volume setting. Held so a stream
  // opened later starts at the right loudness rather than full
  float m_volume = 1.0F;

  // Sized once per stream so the render callback never allocates
  std::vector<float> m_scratch;

  // Generator state for the keepalive noise. Touched only by the audio thread
  uint32_t m_keepAliveNoise = 22695477U;

  // Amplitude of that noise, as a fraction of full scale
  float m_keepAliveAmplitude = 0.0F;

  // Set by the first callback of a stream, to confirm the backend is pulling
  std::atomic_flag m_callbackSeen = ATOMIC_FLAG_INIT;

  // Declared last so it is destroyed first, which stops the audio callback before anything the
  // callback reads goes away
  std::unique_ptr<QAudioSink> m_sink;
};

} // namespace firelight::audio

#include "audio_manager.hpp"

#include <QSettings>
#include <cstring>
#include <spdlog/spdlog.h>

namespace {
// The output device the user picked (by description), persisted in QSettings,
// or the system default if unset / no longer present.
QAudioDevice selectedOutputDevice() {
  const QString desc = QSettings().value("audio/outputDevice").toString();
  if (!desc.isEmpty()) {
    for (const auto &device : QMediaDevices::audioOutputs()) {
      if (device.description() == desc) {
        return device;
      }
    }
  }
  return QMediaDevices::defaultAudioOutput();
}
} // namespace

AudioManager::AudioManager(std::function<void()> onAudioBufferLevelChanged)
  : m_onAudioBufferLevelChanged(std::move(onAudioBufferLevelChanged)) {
  m_mediaDevices = new QMediaDevices(this);
  connect(m_mediaDevices, &QMediaDevices::audioOutputsChanged, this,
          &AudioManager::onAudioDevicesChanged);
}

size_t AudioManager::receive(const int16_t *data, const size_t numFrames) {
  // TODO: REALLY BAD SOLUTION for mupen sometimes sending very small number of
  // frames
  if (numFrames < 30) {
    return numFrames;
  }

  // Note: we run this path even when muted (writing silence below) so the audio
  // buffer keeps draining at the device rate — the "audio" sync method paces the
  // emulation off this buffer's occupancy and must not stall when muted.
  if (m_audioDevice && m_audioSink) {
    // Added m_audioSink check
    const auto bufferTotalCapacity = m_audioSink->bufferSize();
    if (bufferTotalCapacity == 0)
      return numFrames; // Avoid division by zero

    const auto usedBytes = bufferTotalCapacity - m_audioSink->bytesFree();
    m_currentBufferLevel = static_cast<float>(usedBytes) / bufferTotalCapacity;
    if (m_onAudioBufferLevelChanged) {
      m_onAudioBufferLevelChanged();
    }

    // Steer the buffer toward ~50% full by nudging the resample rate.
    const int delta = m_rateController.computeCompensation(
        static_cast<int>(usedBytes), static_cast<int>(bufferTotalCapacity));

    // Resample to the device rate. The resampler applies our drift-compensation
    // delta plus any pending feed-forward playback-rate change.
    std::vector<int16_t> output = m_resampler.process(data, numFrames, delta);
    if (output.empty()) {
      return numFrames; // input consumed, nothing produced this call
    }

    if (m_isMuted) {
      // Keep the buffer flowing (for pacing) but play silence.
      std::memset(output.data(), 0, output.size() * sizeof(int16_t));
    }
    m_audioDevice->write(reinterpret_cast<const char *>(output.data()),
                         output.size() * sizeof(int16_t)); // stereo, s16
  }

  return numFrames; // Return original number of frames consumed
}

void AudioManager::openAudioSink() {
  QAudioFormat format;
  format.setChannelCount(2);
  format.setSampleFormat(QAudioFormat::Int16);
  format.setSampleRate(m_sampleRate);

  m_audioSink = std::make_unique<QAudioSink>(selectedOutputDevice(), format);
  // Larger buffer at higher sample rates.
  const int mult = m_sampleRate > 44000 ? 4 : 2;
  m_audioSink->setBufferSize(8192 * mult);
  m_audioDevice = m_audioSink->start();
}

void AudioManager::initialize(const double new_freq) {
  m_sampleRate = new_freq;
  m_resampler.initialize(m_sampleRate);
  openAudioSink();
}

void AudioManager::setMuted(bool muted) { m_isMuted = muted; }
bool AudioManager::isMuted() const { return m_isMuted; }

float AudioManager::getBufferLevel() const {
  // Live read of the sink's occupancy. The "audio" sync method paces frames off
  // this value from another thread; a cached value only refreshed while we're
  // feeding the sink would freeze once the buffer fills and we stop feeding it,
  // stalling emulation. bytesFree()/bufferSize() are cheap reads.
  if (m_audioSink) {
    const auto capacity = m_audioSink->bufferSize();
    if (capacity > 0) {
      const auto used = capacity - m_audioSink->bytesFree();
      return static_cast<float>(used) / static_cast<float>(capacity);
    }
  }
  return m_currentBufferLevel.load();
}

void AudioManager::setPlaybackRateRatio(double ratio) {
  m_resampler.setPlaybackRateRatio(ratio);
}

void AudioManager::reinitializeAudioDevice() {
  if (!m_audioSink) {
    return; // Not initialized yet
  }

  spdlog::info(
    "Default audio output device changed, reinitializing audio device.");

  if (m_audioDevice) {
    m_audioDevice->close();
    m_audioDevice = nullptr;
  }
  m_audioSink->stop();
  m_audioSink.reset();

  openAudioSink();
}

void AudioManager::onAudioDevicesChanged() { reinitializeAudioDevice(); }

AudioManager::~AudioManager() {
  if (m_audioDevice) {
    m_audioDevice->close();
  }
  if (m_audioSink) {
    m_audioSink->stop();
    m_audioSink.reset();
  }
}

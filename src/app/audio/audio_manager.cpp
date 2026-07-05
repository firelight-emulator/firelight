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
      for (const auto &device: QMediaDevices::audioOutputs()) {
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
  // Previously dropped any batch < 30 frames (a hack for mupen's small batches).
  // Dropping real audio frames leaves a small gap/skip; the resampler buffers
  // tiny inputs fine, so process them instead of discarding.
  if (numFrames == 0) {
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

    // Steer the buffer toward ~50% full by nudging the resample rate, unless the
    // user has disabled Dynamic Rate Control (advanced setting).
    const int delta =
        m_drcEnabled.load()
          ? m_rateController.computeCompensation(
            static_cast<int>(usedBytes),
            static_cast<int>(bufferTotalCapacity))
          : 0;

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

    // Once we've pre-buffered ~half the sink, begin playback. Writes above fill
    // the sink while it's suspended, so playback starts from a healthy buffer.
    if (m_priming) {
      const auto filled = bufferTotalCapacity - m_audioSink->bytesFree();
      if (filled >= bufferTotalCapacity / 2) {
        m_audioSink->resume();
        m_priming = false;
      }
    }
  }

  return numFrames; // Return original number of frames consumed
}

void AudioManager::openAudioSink() {
  const QAudioDevice dev = selectedOutputDevice();

  int deviceRate = dev.preferredFormat().sampleRate();
  if (deviceRate <= 0) {
    deviceRate = m_sampleRate;
  }

  QAudioFormat format;
  format.setChannelCount(2);
  format.setSampleFormat(QAudioFormat::Int16);
  format.setSampleRate(deviceRate);

  if (!dev.isFormatSupported(format)) {
    deviceRate = m_sampleRate;
    format.setSampleRate(deviceRate);
  }

  m_deviceSampleRate = deviceRate;
  spdlog::info("Audio: resampling core {} Hz -> device {} Hz", m_sampleRate,
               m_deviceSampleRate);

  m_resampler.initialize(m_sampleRate, m_deviceSampleRate);

  m_audioSink = std::make_unique<QAudioSink>(dev, format);

  const int bufferMultiplier = m_deviceSampleRate > 44000 ? 4 : 2;
  m_audioSink->setBufferSize(8192 * bufferMultiplier);

  m_audioDevice = m_audioSink->start();

  m_audioSink->suspend();
  m_priming = true;
}

void AudioManager::initialize(const double new_freq) {
  m_sampleRate = static_cast<int>(new_freq);
  openAudioSink();
}

void AudioManager::setMuted(bool muted) { m_isMuted = muted; }
bool AudioManager::isMuted() const { return m_isMuted; }

void AudioManager::setPaused(const bool paused) {
  if (!m_audioSink) {
    return;
  }
  if (paused) {
    m_audioSink->suspend(); // halt playback, keep the buffered audio
  } else if (!m_priming) {
    // While priming the sink is intentionally suspended until pre-buffered;
    // receive() resumes it. Don't let an early unpause start it underrunning.
    m_audioSink->resume();
  }
}

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

void AudioManager::setDynamicRateControlEnabled(const bool enabled) {
  m_drcEnabled.store(enabled);
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

#include "audio_manager.hpp"

#include "audio_device_selection.hpp"

#include <cstring>
#include <spdlog/spdlog.h>

QAudioDevice AudioManager::selectedOutputDevice() const {
  return firelight::audio::selectOutputDevice(m_settingsService);
}

AudioManager::AudioManager(firelight::settings::SettingsService &settingsService,
                           std::function<void()> onAudioBufferLevelChanged)
    : m_settingsService(settingsService), m_onAudioBufferLevelChanged(std::move(onAudioBufferLevelChanged)) {
  m_mediaDevices = new QMediaDevices(this);
  connect(m_mediaDevices, &QMediaDevices::audioOutputsChanged, this, &AudioManager::onAudioDevicesChanged);

  refreshUserMuted();
  refreshVolume();

  // Picking a different output (or muting, or moving the slider) applies
  // straight away, the same as the device list changing under us
  const auto onKey = [this](const std::string &key) {
    if (key == OUTPUT_DEVICE_KEY) {
      reinitializeAudioDevice();
    } else if (key == MUTED_KEY) {
      refreshUserMuted();
    } else if (key == VOLUME_KEY) {
      refreshVolume();
    }
  };
  m_settingChangedConnection = EventDispatcher::instance().subscribe<firelight::settings::GlobalSettingChangedEvent>(
      [onKey](const firelight::settings::GlobalSettingChangedEvent &e) { onKey(e.key); });
  m_settingResetConnection = EventDispatcher::instance().subscribe<firelight::settings::GlobalSettingResetEvent>(
      [onKey](const firelight::settings::GlobalSettingResetEvent &e) { onKey(e.key); });
}

void AudioManager::refreshUserMuted() {
  m_userMuted = m_settingsService.getGlobalValue(MUTED_KEY).value_or("false") == "true";
}

void AudioManager::refreshVolume() {
  auto percent = 100;
  try {
    percent = std::stoi(m_settingsService.getGlobalValue(VOLUME_KEY).value_or("100"));
  } catch (const std::exception &) {
    percent = 100; // missing or non-numeric -> full, not silent
  }
  m_volume = std::clamp(percent, 0, 100) / 100.0f;

  std::lock_guard lock(m_sinkMutex);
  if (m_audioSink) {
    // Perceptual, not linear: halfway along the slider should sound halfway,
    // and a linear 0.5 doesn't
    m_audioSink->setVolume(
        QtAudio::convertVolume(m_volume.load(), QtAudio::LogarithmicVolumeScale, QtAudio::LinearVolumeScale));
  }
}

size_t AudioManager::receive(const int16_t *data, const size_t numFrames) {
  if (numFrames == 0) {
    return numFrames;
  }

  // Note: we run this path even when muted (writing silence below) so the audio
  // buffer keeps draining at the device rate — the "audio" sync method paces the
  // emulation off this buffer's occupancy and must not stall when muted
  std::lock_guard lock(m_sinkMutex);
  if (m_audioDevice && m_audioSink) {
    // Added m_audioSink check
    const auto bufferTotalCapacity = m_audioSink->bufferSize();
    if (bufferTotalCapacity == 0) {
      return numFrames; // Avoid division by zero
    }

    const auto usedBytes = bufferTotalCapacity - m_audioSink->bytesFree();
    m_currentBufferLevel = static_cast<float>(usedBytes) / bufferTotalCapacity;
    if (m_onAudioBufferLevelChanged) {
      m_onAudioBufferLevelChanged();
    }

    // Steer the buffer toward ~50% full by nudging the resample rate, unless the
    // user has disabled Dynamic Rate Control (advanced setting)
    const int delta = m_drcEnabled.load() ? m_rateController.computeCompensation(static_cast<int>(usedBytes),
                                                                                 static_cast<int>(bufferTotalCapacity))
                                          : 0;

    std::vector<int16_t> output = m_resampler.process(data, numFrames, delta);
    if (output.empty()) {
      return numFrames; // input consumed, nothing produced this call
    }

    if (m_isMuted || m_userMuted) {
      // Keep the buffer flowing (for pacing) but play silence
      std::memset(output.data(), 0, output.size() * sizeof(int16_t));
    }
    m_audioDevice->write(reinterpret_cast<const char *>(output.data()),
                         output.size() * sizeof(int16_t)); // stereo, s16

    // Once we've pre-buffered ~half the sink, begin playback. Writes above fill
    // the sink while it's suspended, so playback starts from a healthy buffer
    if (m_priming) {
      const auto filled = bufferTotalCapacity - m_audioSink->bytesFree();
      if (filled >= bufferTotalCapacity / 2) {
        m_audioSink->resume();
        m_priming = false;
      }
    }
  }

  return numFrames;
}

// Callers (initialize / reinitializeAudioDevice) hold m_sinkMutex
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
  spdlog::info("Audio: resampling core {} Hz -> device {} Hz", m_sampleRate, m_deviceSampleRate);

  // Initialize the resampler to convert from the core's sample rate to the device's sample rate
  m_resampler.initialize(m_sampleRate, m_deviceSampleRate);

  m_audioSink = std::make_unique<QAudioSink>(dev, format);
  m_audioSink->setVolume(
      QtAudio::convertVolume(m_volume.load(), QtAudio::LogarithmicVolumeScale, QtAudio::LinearVolumeScale));

  // Set a larger buffer for higher sample rates
  const int bufferMultiplier = m_deviceSampleRate > 44000 ? 4 : 2;
  m_audioSink->setBufferSize(8192 * bufferMultiplier);

  m_audioDevice = m_audioSink->start();

  // Suspend the sink until we've pre-buffered to around 50% full
  m_audioSink->suspend();
  m_priming = true;
}

void AudioManager::initialize(const double new_freq) {
  std::lock_guard lock(m_sinkMutex);
  m_sampleRate = static_cast<int>(new_freq);
  openAudioSink();
}

void AudioManager::setMuted(bool muted) { m_isMuted = muted; }

bool AudioManager::isMuted() const { return m_isMuted; }

void AudioManager::setPaused(const bool paused) {
  std::lock_guard lock(m_sinkMutex);
  if (!m_audioSink) {
    return;
  }
  if (paused) {
    m_audioSink->suspend(); // halt playback, keep the buffered audio
  } else if (!m_priming) {
    // While priming the sink is intentionally suspended until pre-buffered;
    // receive() resumes it. Don't let an early unpause start it underrunning
    m_audioSink->resume();
  }
}

float AudioManager::getBufferLevel() const {
  // Live read of the sink's occupancy. The "audio" sync method paces frames off
  // this value from another thread; a cached value only refreshed while we're
  // feeding the sink would freeze once the buffer fills and we stop feeding it,
  // stalling emulation. bytesFree()/bufferSize() are cheap reads, but the sink
  // isn't thread-safe, so serialize against receive()/reinit on other threads
  std::lock_guard lock(m_sinkMutex);
  if (m_audioSink) {
    const auto capacity = m_audioSink->bufferSize();
    if (capacity > 0) {
      const auto used = capacity - m_audioSink->bytesFree();
      return static_cast<float>(used) / static_cast<float>(capacity);
    }
  }
  return m_currentBufferLevel.load();
}

void AudioManager::setPlaybackRateRatio(double ratio) { m_resampler.setPlaybackRateRatio(ratio); }

void AudioManager::setDynamicRateControlEnabled(const bool enabled) { m_drcEnabled.store(enabled); }

void AudioManager::reinitializeAudioDevice() {
  std::lock_guard lock(m_sinkMutex);
  if (!m_audioSink) {
    return; // Not initialized yet
  }

  spdlog::info("Default audio output device changed, reinitializing audio device.");

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
  std::lock_guard lock(m_sinkMutex);
  if (m_audioDevice) {
    m_audioDevice->close();
  }
  if (m_audioSink) {
    m_audioSink->stop();
    m_audioSink.reset();
  }
}

#include "audio_manager.hpp"

#include "audio_output_stream.hpp"
#include "diagnostics/performance_stats.hpp"

#include <QCoreApplication>
#include <QThread>
#include <algorithm>
#include <cstring>
#include <spdlog/spdlog.h>
#include <vector>

using firelight::audio::PlaybackBuffer;

AudioManager::AudioManager(firelight::settings::SettingsService &settingsService, std::string contentHash,
                           const int platformId, std::function<void()> onAudioBufferLevelChanged)
    : m_settingsService(settingsService), m_onAudioBufferLevelChanged(std::move(onAudioBufferLevelChanged)),
      m_buffer(std::make_shared<PlaybackBuffer>()) {
  m_stream = new firelight::audio::AudioOutputStream(settingsService, std::move(contentHash), platformId, m_buffer);

  // Built on whichever thread loads the game, pushed to the GUI thread from there
  if (const auto *app = QCoreApplication::instance(); app && app->thread() != QThread::currentThread()) {
    m_stream->moveToThread(app->thread());
  }

  refreshUserMuted();

  const auto onKey = [this](const std::string &key) {
    if (key == MUTED_KEY) {
      refreshUserMuted();
    }
  };
  m_settingChangedConnection = EventDispatcher::instance().subscribe<firelight::settings::GlobalSettingChangedEvent>(
      [onKey](const firelight::settings::GlobalSettingChangedEvent &e) { onKey(e.key); });
  m_settingResetConnection = EventDispatcher::instance().subscribe<firelight::settings::GlobalSettingResetEvent>(
      [onKey](const firelight::settings::GlobalSettingResetEvent &e) { onKey(e.key); });
}

AudioManager::~AudioManager() {
  if (QThread::currentThread() == m_stream->thread()) {
    delete m_stream;
  } else {
    m_stream->deleteLater();
  }
}

void AudioManager::refreshUserMuted() {
  m_userMuted = m_settingsService.getGlobalValue(MUTED_KEY).value_or("false") == "true";
}

size_t AudioManager::receive(const int16_t *data, const size_t numFrames) {
  if (numFrames == 0) {
    return numFrames;
  }

  const firelight::monitoring::ScopedSpan batchSpan(m_audioBatchSpan);

  const auto deviceRate = m_stream->getSampleRate();

  if (deviceRate <= 0 || m_sampleRate <= 0) {
    return numFrames;
  }

  if (deviceRate != m_deviceSampleRate) {
    m_deviceSampleRate = deviceRate;
    spdlog::info("Audio: resampling core {} Hz -> output {} Hz", m_sampleRate, m_deviceSampleRate);
    m_resampler.initialize(m_sampleRate, m_deviceSampleRate);
    m_rateController.reset();
  }

  const auto capacityFrames = m_buffer->getCapacityFrames();
  const auto usedFrames = m_buffer->getSizeFrames();
  const auto occupancy = m_buffer->getOccupancy();

  // Negative while the buffer is filling before playback starts, when its level says nothing about drift
  const auto measurable = occupancy >= 0.0;

  if (measurable) {
    m_audioBufferSeries.record(occupancy);

    if (m_onAudioBufferLevelChanged) {
      m_onAudioBufferLevelChanged();
    }
  }

  // Converted to the output's rate so the controller's smoothing spans a fixed amount of sound rather than a fixed
  // number of calls
  const auto framesAtOutputRate =
      static_cast<int>(numFrames * static_cast<size_t>(m_deviceSampleRate) / static_cast<size_t>(m_sampleRate));

  constexpr auto BYTES_PER_FRAME = static_cast<int>(PlaybackBuffer::CHANNELS * sizeof(int16_t));
  const double compensation =
      m_drcEnabled.load() && measurable
          ? m_rateController.computeCompensation(static_cast<int>(usedFrames) * BYTES_PER_FRAME,
                                                 static_cast<int>(capacityFrames) * BYTES_PER_FRAME, framesAtOutputRate)
          : 0.0;
  m_audioCorrectionSeries.record(compensation);

  std::vector<int16_t> output;
  {
    const firelight::monitoring::ScopedSpan resampleSpan(m_resampleSpan);
    output = m_resampler.process(data, numFrames, compensation);
  }

  if (output.empty()) {
    return numFrames;
  }

  if (m_isMuted || m_userMuted) {
    // Keep the buffer flowing (for pacing) but play silence
    std::memset(output.data(), 0, output.size() * sizeof(int16_t));
  }

  const auto frames = output.size() / PlaybackBuffer::CHANNELS;
  m_buffer->push(output.data(), frames);

  firelight::diagnostics::PerformanceStats::instance().recordAudio(static_cast<double>(usedFrames) /
                                                                       static_cast<double>(capacityFrames),
                                                                   compensation, static_cast<int64_t>(frames));
  m_audioSamplesSeries.record(static_cast<double>(frames));

  return numFrames;
}

void AudioManager::initialize(const double newFreq) {
  m_sampleRate = static_cast<int>(newFreq);
  m_deviceSampleRate = 0;
  m_stream->open(m_sampleRate);
}

void AudioManager::setMuted(const bool muted) { m_isMuted = muted; }

bool AudioManager::isMuted() const { return m_isMuted; }

void AudioManager::setPaused(const bool paused) { m_buffer->setPaused(paused); }

float AudioManager::getBufferLevel() const { return static_cast<float>(m_buffer->getOccupancy()); }

void AudioManager::setPlaybackRateRatio(const double ratio) { m_resampler.setPlaybackRateRatio(ratio); }

void AudioManager::setDynamicRateControlEnabled(const bool enabled) { m_drcEnabled.store(enabled); }

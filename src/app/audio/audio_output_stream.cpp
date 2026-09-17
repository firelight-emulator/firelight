// TODO: NEEDS REVIEW
#include "audio_output_stream.hpp"

#include "audio_device_selection.hpp"
#include "audio_settings.hpp"
#include "diagnostics/performance_stats.hpp"

#include <QAudioFormat>
#include <QSpan>
#include <algorithm>
#include <spdlog/spdlog.h>

namespace firelight::audio {

namespace {
// TODO
// Device enumeration and settings arrive in bursts, so a reopen waits for the burst to settle
constexpr int REOPEN_DEBOUNCE_MS = 250;
constexpr int MAX_REOPEN_BACKOFF_MS = 10000;
} // namespace

AudioOutputStream::AudioOutputStream(settings::SettingsService &settingsService, std::string contentHash,
                                     const int platformId, std::shared_ptr<PlaybackBuffer> buffer)
    : m_settingsService(settingsService), m_contentHash(std::move(contentHash)), m_platformId(platformId),
      m_buffer(std::move(buffer)) {
  m_mediaDevices = new QMediaDevices(this);
  connect(m_mediaDevices, &QMediaDevices::audioOutputsChanged, this, &AudioOutputStream::onAudioOutputsChanged);

  m_reopenTimer = new QTimer(this);
  m_reopenTimer->setSingleShot(true);
  m_reopenBackoffMs = REOPEN_DEBOUNCE_MS;
  connect(m_reopenTimer, &QTimer::timeout, this, &AudioOutputStream::reopen);

  refreshVolume();

  // TODO
  // Settings events arrive on whichever thread changed the setting
  const auto onKey = [this](const std::string &key) {
    if (key == OUTPUT_DEVICE_KEY || key == LATENCY_KEY) {
      QMetaObject::invokeMethod(this, [this] { scheduleReopen(REOPEN_DEBOUNCE_MS); }, Qt::QueuedConnection);
    } else if (key == VOLUME_KEY) {
      QMetaObject::invokeMethod(this, [this] { refreshVolume(); }, Qt::QueuedConnection);
    }
  };
  m_settingChangedConnection = EventDispatcher::instance().subscribe<settings::GlobalSettingChangedEvent>(
      [onKey](const settings::GlobalSettingChangedEvent &e) { onKey(e.key); });
  m_settingResetConnection = EventDispatcher::instance().subscribe<settings::GlobalSettingResetEvent>(
      [onKey](const settings::GlobalSettingResetEvent &e) { onKey(e.key); });
}

AudioOutputStream::~AudioOutputStream() { m_sink.reset(); }

void AudioOutputStream::open(const int coreSampleRate) {
  QMetaObject::invokeMethod(
      this,
      [this, coreSampleRate] {
        m_coreSampleRate = coreSampleRate;
        reopen();
      },
      Qt::QueuedConnection);
}

int AudioOutputStream::getSampleRate() const { return m_sampleRate.load(std::memory_order_acquire); }

void AudioOutputStream::onAudioOutputsChanged() { scheduleReopen(REOPEN_DEBOUNCE_MS); }

void AudioOutputStream::onSinkStateChanged(const QtAudio::State state) {
  if (state != QtAudio::StoppedState || !m_sink || m_sink->error() == QtAudio::NoError) {
    return;
  }

  spdlog::warn("Audio output stopped with error {}; reopening", static_cast<int>(m_sink->error()));
  m_sampleRate.store(0, std::memory_order_release);
  scheduleReopen(m_reopenBackoffMs);
  m_reopenBackoffMs = std::min(m_reopenBackoffMs * 2, MAX_REOPEN_BACKOFF_MS);
}

void AudioOutputStream::scheduleReopen(const int delayMs) { m_reopenTimer->start(delayMs); }

void AudioOutputStream::reopen() {
  if (m_coreSampleRate <= 0) {
    return;
  }

  m_sink.reset();

  const auto fail = [this](const std::string &why) {
    spdlog::warn("Audio: {}; retrying in {} ms", why, m_reopenBackoffMs);
    m_sampleRate.store(0, std::memory_order_release);
    m_buffer->restart(m_buffer->getCapacityFrames(), m_buffer->getCapacityFrames());
    scheduleReopen(m_reopenBackoffMs);
    m_reopenBackoffMs = std::min(m_reopenBackoffMs * 2, MAX_REOPEN_BACKOFF_MS);
  };

  const auto device = selectOutputDevice(m_settingsService);

  if (device.isNull()) {
    fail("no output device");
    return;
  }

  QAudioFormat format;
  format.setChannelCount(static_cast<int>(PlaybackBuffer::CHANNELS));
  format.setSampleFormat(QAudioFormat::Int16);
  format.setSampleRate(device.preferredFormat().sampleRate() > 0 ? device.preferredFormat().sampleRate()
                                                                 : m_coreSampleRate);

  if (!device.isFormatSupported(format)) {
    format.setSampleRate(m_coreSampleRate);
  }

  if (!device.isFormatSupported(format)) {
    fail("output '" + device.description().toStdString() + "' takes no usable format");
    return;
  }

  const auto rate = format.sampleRate();
  const auto capacityFrames = static_cast<size_t>(latencyMs()) * static_cast<size_t>(rate) / 1000;
  m_buffer->restart(capacityFrames, static_cast<size_t>(static_cast<double>(capacityFrames) * PRIMING_FILL_FRACTION));

  m_sink = std::make_unique<QAudioSink>(device, format);
  m_sink->setVolume(m_volume);
  connect(m_sink.get(), &QAudioSink::stateChanged, this, &AudioOutputStream::onSinkStateChanged, Qt::QueuedConnection);
  m_sink->start([buffer = m_buffer](QSpan<int16_t> out) {
    buffer->render(out.data(), static_cast<size_t>(out.size()) / PlaybackBuffer::CHANNELS);
  });

  if (m_sink->error() != QtAudio::NoError) {
    const auto error = static_cast<int>(m_sink->error());
    m_sink.reset();
    fail("output stream failed to start (error " + std::to_string(error) + ")");
    return;
  }

  m_sampleRate.store(rate, std::memory_order_release);
  m_reopenBackoffMs = REOPEN_DEBOUNCE_MS;

  const auto capacityBytes = static_cast<int>(capacityFrames * PlaybackBuffer::CHANNELS * sizeof(int16_t));
  diagnostics::PerformanceStats::instance().setAudioDevice(device.description().toStdString(), capacityBytes,
                                                           m_coreSampleRate);
  spdlog::info("Audio: core {} Hz -> '{}' at {} Hz, {} ms buffered ({} frames)", m_coreSampleRate,
               device.description().toStdString(), rate, latencyMs(), capacityFrames);
}

void AudioOutputStream::refreshVolume() {
  auto percent = 100;

  try {
    percent = std::stoi(m_settingsService.getGlobalValue(VOLUME_KEY).value_or("100"));
  } catch (const std::exception &) {
    percent = 100;
  }

  m_volume = QtAudio::convertVolume(std::clamp(percent, 0, 100) / 100.0F, QtAudio::LogarithmicVolumeScale,
                                    QtAudio::LinearVolumeScale);

  if (m_sink) {
    m_sink->setVolume(m_volume);
  }
}

int AudioOutputStream::latencyMs() const {
  try {
    return std::clamp(
        std::stoi(m_settingsService.getEffectiveValue(m_contentHash, m_platformId, LATENCY_KEY).value_or("")),
        MIN_LATENCY_MS, MAX_LATENCY_MS);
  } catch (const std::exception &) {
    return DEFAULT_LATENCY_MS;
  }
}

} // namespace firelight::audio

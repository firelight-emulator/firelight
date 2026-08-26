// TODO: NEEDS REVIEW
#include "audio_manager.hpp"

#include "audio_device_selection.hpp"
#include "diagnostics/performance_stats.hpp"

#include <firelight/audio/audio_rate_controller.hpp>

#include <QtGlobal>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstring>
#include <spdlog/spdlog.h>
#include <string>

namespace {
// How long to wait between attempts at rebuilding a lost output, doubling up to the cap so one
// that is simply gone is not retried on every frame
constexpr int REOPEN_DELAY_MS = 250;
constexpr int MAX_REOPEN_BACKOFF_MS = 10000;

// FLDRC (temporary instrumentation — remove this block and its call in receive())
// What rate control is actually doing on real hardware, which the simulation cannot show: whether
// occupancy holds where the gain says it should, whether the correction stays inaudible, and what
// the device's true rate turns out to be.
//
// The rate comes from the slope of the sink's cumulative counter against a monotonic clock. That
// counter only moves in whole device periods — 10.67 ms on CoreAudio — but quantisation is noise on
// a cumulative quantity, so the slope recovers the real rate to a few parts per million anyway
class DrcDiagnostics {
public:
  void record(const double occupancy, const double requestedRatio, const int64_t processedUSecs,
              const int64_t offeredBytes, const int64_t takenBytes, const int64_t backlogBytes,
              const int64_t batchBytes, const int64_t droppedBytes) {
    const auto nowNs = std::chrono::steady_clock::now().time_since_epoch().count();

    // A new sink restarts the counter from zero while the wall clock carries on, which would read as
    // an enormous rate error for as long as the baseline stood. Compared against the LAST reading
    // rather than the baseline, because the baseline is itself near zero at startup and a restart to
    // zero would never look like a decrease
    if (m_startNs == 0 || processedUSecs < m_lastUSecs) {
      m_startNs = nowNs;
      m_startUSecs = processedUSecs;
      m_lastUSecs = processedUSecs;
      m_reportAtNs = nowNs + reportIntervalNs();
      return;
    }

    m_lastUSecs = processedUSecs;

    m_buckets[std::clamp(static_cast<int>(occupancy * BUCKETS), 0, BUCKETS - 1)]++;
    m_occupancySum += occupancy;
    m_ratioSum += requestedRatio;
    m_ratioPeak = std::max(m_ratioPeak, std::abs(requestedRatio));
    m_count++;

    if (occupancy <= 0.001) {
      m_dry++;
    } else if (occupancy >= 0.999) {
      m_full++;
    }

    // TODO
    // A refusal is the sink taking less than it was offered, which is the only thing that puts audio
    // in the backlog. Counted per call rather than sampled, so it cannot be missed between reports
    if (takenBytes < offeredBytes) {
      m_refusals++;
      m_refusedBytes += offeredBytes - takenBytes;
    }

    m_backlogPeak = std::max(m_backlogPeak, backlogBytes);
    m_backlogSum += static_cast<double>(backlogBytes);
    m_batchPeak = std::max(m_batchPeak, batchBytes);
    m_droppedBytes += droppedBytes;

    if (nowNs < m_reportAtNs) {
      return;
    }

    const auto elapsedSecs = static_cast<double>(nowNs - m_startNs) / 1e9;
    const auto processedSecs = static_cast<double>(processedUSecs - m_startUSecs) / 1e6;
    const auto rateRatio = elapsedSecs > 0.0 ? processedSecs / elapsedSecs : 0.0;

    std::string shape;
    for (auto i = 0; i < BUCKETS; ++i) {
      shape += fmt::format(" {}0%:{}", i, m_buckets[i]);
    }

    // A starved sink cannot consume at its own rate, so while it is running dry this figure measures
    // the starvation rather than the device and must not be read as a crystal error
    const auto rateIsMeaningful = m_dry == 0;

    spdlog::info("FLDRC occupancy mean={:.1f}% |{} | correction mean={:+.4f}% peak={:.4f}% | dry={} full={} | {}",
                 m_occupancySum / m_count * 100.0, shape, m_ratioSum / m_count * 100.0, m_ratioPeak * 100.0, m_dry,
                 m_full,
                 rateIsMeaningful ? fmt::format("device rate {:.6f}x nominal ({:+.1f} ppm over {:.0f}s)", rateRatio,
                                                (rateRatio - 1.0) * 1e6, elapsedSecs)
                                  : std::string("device rate not measurable while the sink is running dry"));

    // TODO
    // The sink's own occupancy says nothing about audio held back from it. Capacity, the largest
    // batch a core handed over, and what the backlog grew to are the three numbers that say whether
    // a buffer is big enough for the core feeding it
    spdlog::info("FLBUF capacity={}B ({:.1f}ms) | biggest batch={}B ({:.1f}ms, {:.0f}% of capacity) | "
                 "refusals={} refused={}B | backlog mean={:.1f}ms peak={:.1f}ms | dropped={}B ({:.1f}ms)",
                 m_capacityBytes, msFor(m_capacityBytes), m_batchPeak, msFor(m_batchPeak),
                 m_capacityBytes > 0 ? 100.0 * m_batchPeak / m_capacityBytes : 0.0, m_refusals, m_refusedBytes,
                 msFor(static_cast<int64_t>(m_backlogSum / std::max(m_count, 1))), msFor(m_backlogPeak), m_droppedBytes,
                 msFor(m_droppedBytes));

    m_refusals = 0;
    m_refusedBytes = 0;
    m_backlogPeak = 0;
    m_backlogSum = 0.0;
    m_batchPeak = 0;
    m_droppedBytes = 0;

    m_buckets.fill(0);
    m_occupancySum = 0.0;
    m_ratioSum = 0.0;
    m_ratioPeak = 0.0;
    m_count = 0;
    m_dry = 0;
    m_full = 0;
    m_reportAtNs = nowNs + reportIntervalNs();
  }

  // TODO
  // Anything that would be heard, reported when it happens rather than averaged into the next
  // window. A buffer at either end, or a correction with nothing left to give, is the shape every
  // audible episode so far has had. Rate limited, because the interesting part is when one starts
  void noteIfAudible(const double occupancy, const double correction, const int64_t backlogBytes,
                     const int64_t refusedBytes) {
    const auto saturated = std::abs(correction) >= AudioRateController::MAX_CORRECTION * 0.99;
    const auto atTheEnds = occupancy >= 0.97 || occupancy <= 0.03;

    if (!saturated && !atTheEnds) {
      m_wasAudible = false;
      return;
    }

    const auto nowNs = std::chrono::steady_clock::now().time_since_epoch().count();
    if (m_wasAudible && nowNs < m_nextNoteAtNs) {
      return;
    }

    spdlog::warn("FLPOP occupancy={:.1f}% correction={:+.4f}%{} | backlog={:.1f}ms refused={}B this call",
                 occupancy * 100.0, correction * 100.0, saturated ? " (SATURATED)" : "", msFor(backlogBytes),
                 refusedBytes);

    m_wasAudible = true;
    m_nextNoteAtNs = nowNs + 250000000LL;
  }

  // TODO
  // Capacity and rate are fixed for the life of a sink and change together when one is rebuilt
  void describeSink(const int capacityBytes, const int deviceRate) {
    m_capacityBytes = capacityBytes;
    m_deviceRate = deviceRate;
  }

private:
  static constexpr int BUCKETS = 10;

  // FL_DIAG_SECS tunes how often this interrupts the thread it runs on. Writing a line blocks the
  // render thread for as long as the write takes, so a run being judged for hitching wants this rare
  static int64_t reportIntervalNs() {
    const auto secs = qEnvironmentVariableIntValue("FL_DIAG_SECS");
    return (secs > 0 ? secs : 10) * 1000000000LL;
  }

  // TODO
  // Bytes are what every one of these is measured in, and milliseconds are what anyone reading the
  // line wants. Set once the sink is open, since the rate is not known before that
  int m_deviceRate = 48000;

  [[nodiscard]] double msFor(const int64_t bytes) const {
    return m_deviceRate > 0 ? static_cast<double>(bytes) * 1000.0 / (m_deviceRate * 4) : 0.0;
  }

  bool m_wasAudible = false;
  int64_t m_nextNoteAtNs = 0;
  int64_t m_capacityBytes = 0;
  int64_t m_refusals = 0;
  int64_t m_refusedBytes = 0;
  int64_t m_backlogPeak = 0;
  double m_backlogSum = 0.0;
  int64_t m_batchPeak = 0;
  int64_t m_droppedBytes = 0;

  std::array<int, BUCKETS> m_buckets{};
  double m_occupancySum = 0.0;
  double m_ratioSum = 0.0;
  double m_ratioPeak = 0.0;
  int m_count = 0;
  int m_dry = 0;
  int m_full = 0;
  int64_t m_startNs = 0;
  int64_t m_startUSecs = 0;
  int64_t m_lastUSecs = 0;
  int64_t m_reportAtNs = 0;
};

DrcDiagnostics drcDiagnostics; // FLDRC (temporary)
} // namespace

QAudioDevice AudioManager::selectedOutputDevice() const {
  return firelight::audio::selectOutputDevice(m_settingsService);
}

AudioManager::AudioManager(firelight::settings::SettingsService &settingsService,
                           std::function<void()> onAudioBufferLevelChanged)
    : m_settingsService(settingsService), m_onAudioBufferLevelChanged(std::move(onAudioBufferLevelChanged)) {
  m_mediaDevices = new QMediaDevices(this);
  connect(m_mediaDevices, &QMediaDevices::audioOutputsChanged, this, &AudioManager::onAudioDevicesChanged);

  m_reopenBackoffMs = REOPEN_DELAY_MS;

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
    } else if (key == LATENCY_KEY) {
      // The buffer's size is fixed when the sink is created, so a new one has to be built for it
      reinitializeAudioDevice();
    }
  };
  m_settingChangedConnection = EventDispatcher::instance().subscribe<firelight::settings::GlobalSettingChangedEvent>(
      [onKey](const firelight::settings::GlobalSettingChangedEvent &e) { onKey(e.key); });
  m_settingResetConnection = EventDispatcher::instance().subscribe<firelight::settings::GlobalSettingResetEvent>(
      [onKey](const firelight::settings::GlobalSettingResetEvent &e) { onKey(e.key); });
}

int AudioManager::latencyMs() const {
  try {
    return std::clamp(std::stoi(m_settingsService.getGlobalValue(LATENCY_KEY).value_or("")),
                      firelight::audio::MIN_LATENCY_MS, firelight::audio::MAX_LATENCY_MS);
  } catch (const std::exception &) {
    return firelight::audio::DEFAULT_LATENCY_MS;
  }
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

  // Checked here rather than from the sink's own signal: this object lives on the render thread,
  // and receive() is the one thing guaranteed to run there
  recoverFromDeviceLoss();

  if (m_audioDevice && isSinkWritable()) {
    const auto bufferTotalCapacity = m_audioSink->bufferSize();
    if (bufferTotalCapacity == 0) {
      return numFrames; // Avoid division by zero
    }

    const auto usedBytes = bufferTotalCapacity - m_audioSink->bytesFree();

    // TODO
    // Only published while the sink is running. Measured on Qt 6.11, bytesFree() does report real
    // occupancy while suspended, so this is not the "answers zero" hazard an older comment here
    // claimed — it is that a suspended sink is one being primed, and its occupancy is on its way up
    // by design rather than telling anyone anything about drift
    const auto measurable = isSinkMeasurable();

    if (measurable) {
      m_currentBufferLevel = static_cast<float>(usedBytes) / bufferTotalCapacity;
      if (m_onAudioBufferLevelChanged) {
        m_onAudioBufferLevelChanged();
      }
    }

    // Steer the buffer toward ~50% full by nudging the resample rate, unless the
    // user has disabled Dynamic Rate Control (advanced setting)
    // TODO
    // Converted to the sink's rate so the controller's smoothing spans a fixed amount of sound rather
    // than a fixed number of calls, which is whatever the core felt like
    const auto framesAtSinkRate = m_sampleRate > 0 ? static_cast<int>(numFrames * m_deviceSampleRate / m_sampleRate)
                                                   : static_cast<int>(numFrames);

    // TODO
    // Gated the same way. Every game starts with the sink suspended while the buffer fills from
    // empty, and a controller told about that would spend the opening seconds correcting for a climb
    // that the priming loop is causing on purpose
    const double compensation =
        m_drcEnabled.load() && measurable
            ? m_rateController.computeCompensation(static_cast<int>(usedBytes), static_cast<int>(bufferTotalCapacity),
                                                   framesAtSinkRate)
            : 0.0;

    std::vector<int16_t> output = m_resampler.process(data, numFrames, compensation);
    if (output.empty()) {
      return numFrames; // input consumed, nothing produced this call
    }

    if (m_isMuted || m_userMuted) {
      // Keep the buffer flowing (for pacing) but play silence
      std::memset(output.data(), 0, output.size() * sizeof(int16_t));
    }

    const auto *bytes = reinterpret_cast<const char *>(output.data());
    const auto batchBytes = static_cast<int64_t>(output.size() * sizeof(int16_t));
    m_pendingBytes.insert(m_pendingBytes.end(), bytes, bytes + batchBytes);

    const auto offeredBytes = static_cast<int64_t>(m_pendingBytes.size());
    const auto taken = m_audioDevice->write(m_pendingBytes.data(), static_cast<qint64>(offeredBytes));
    if (taken > 0) {
      m_pendingBytes.erase(m_pendingBytes.begin(), m_pendingBytes.begin() + taken);
    }

    // TODO
    // Bounded after the write rather than before it, so the cap applies to what the sink actually
    // refused. Clamping the queue first throws away the head of the very run the sink was about to
    // take, which is a splice in the waveform on the call that was recovering
    int64_t droppedBytes = 0;
    if (m_pendingBytes.size() > static_cast<size_t>(bufferTotalCapacity)) {
      droppedBytes = static_cast<int64_t>(m_pendingBytes.size()) - bufferTotalCapacity;
      m_pendingBytes.erase(m_pendingBytes.begin(), m_pendingBytes.end() - bufferTotalCapacity);
    }

    firelight::diagnostics::PerformanceStats::instance().recordAudio(
        static_cast<double>(usedBytes) / bufferTotalCapacity, compensation, static_cast<int64_t>(output.size() / 2));

    // FLPOP / FLDRC / FLBUF (temporary)
    // TODO
    // Silent while priming, because a buffer being filled from empty on purpose passes through both
    // ends on its way to the level it is being filled to
    if (!m_priming) {
      drcDiagnostics.noteIfAudible(static_cast<double>(usedBytes) / bufferTotalCapacity, compensation,
                                   static_cast<int64_t>(m_pendingBytes.size()),
                                   offeredBytes - std::max<int64_t>(taken, 0));
    }
    drcDiagnostics.record(static_cast<double>(usedBytes) / bufferTotalCapacity, compensation,
                          m_audioSink->processedUSecs(), offeredBytes, std::max<int64_t>(taken, 0),
                          static_cast<int64_t>(m_pendingBytes.size()), batchBytes, droppedBytes);

    // Once we've pre-buffered enough of the sink, begin playback. Writes above fill
    // the sink while it's suspended, so playback starts from a healthy buffer
    if (m_priming) {
      const auto filled = bufferTotalCapacity - m_audioSink->bytesFree();
      if (filled >= static_cast<qint64>(bufferTotalCapacity * firelight::audio::PRIMING_FILL_FRACTION)) {
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

  // TODO
  // Sized by how much sound the user wants buffered ahead rather than by a constant, so the figure
  // means the same thing at any device rate. It is a latency the player feels directly: everything
  // written is heard this far in the future, and there are ten frames of it at the old 170 ms
  m_audioSink->setBufferSize(firelight::audio::bufferBytesForLatency(latencyMs(), m_deviceSampleRate));

  m_audioDevice = m_audioSink->start();

  // TODO
  // Read back rather than assumed: the request above is a hint and the sink is free to round it
  spdlog::info("Audio: sink buffer requested {} ms, got {} bytes ({:.1f} ms)", latencyMs(), m_audioSink->bufferSize(),
               m_deviceSampleRate > 0 ? m_audioSink->bufferSize() * 1000.0 / (m_deviceSampleRate * 4) : 0.0);
  drcDiagnostics.describeSink(m_audioSink->bufferSize(), m_deviceSampleRate);
  firelight::diagnostics::PerformanceStats::instance().setAudioDevice(dev.description().toStdString(),
                                                                      m_audioSink->bufferSize(), m_sampleRate);

  // Suspend the sink until we've pre-buffered to around 50% full
  m_audioSink->suspend();
  m_priming = true;

  m_pendingBytes.clear();

  // TODO
  // The cached level belongs to the sink just discarded, and pacing reads it whenever the live one
  // cannot be read. Left alone it would report the old sink's occupancy for as long as the new one
  // is priming
  m_currentBufferLevel = -1.0f;

  // TODO
  // What the controller had smoothed was occupancy of the buffer just discarded, in bytes of a
  // capacity that may not be the same again
  m_rateController.reset();
}

// TODO
// The sink stops itself when the output it was running on goes away — a driver restarting, a rate
// changed in the system's sound settings, another application taking the device. The machine's
// list of outputs does not change, so nothing else notices, and the device start() handed back
// dies with it
void AudioManager::recoverFromDeviceLoss() {
  if (!m_audioSink || m_audioSink->state() != QtAudio::StoppedState || m_audioSink->error() == QtAudio::NoError) {
    return;
  }

  // Dropped first and unconditionally: writing to it is what this exists to prevent, whether or
  // not the rebuild below is due yet
  m_audioDevice = nullptr;

  const auto now = std::chrono::steady_clock::now();
  if (now < m_nextReopenAttempt) {
    return;
  }

  spdlog::warn("Audio output stopped with error {}; reopening", static_cast<int>(m_audioSink->error()));

  m_nextReopenAttempt = now + std::chrono::milliseconds(m_reopenBackoffMs);

  m_audioSink->stop();
  m_audioSink.reset();
  openAudioSink();

  if (isSinkWritable()) {
    m_reopenBackoffMs = REOPEN_DELAY_MS;
  } else {
    m_reopenBackoffMs = std::min(m_reopenBackoffMs * 2, MAX_REOPEN_BACKOFF_MS);
  }
}

bool AudioManager::isSinkWritable() const { return m_audioSink && m_audioSink->state() != QtAudio::StoppedState; }

bool AudioManager::isSinkMeasurable() const {
  if (!m_audioSink) {
    return false;
  }

  const auto state = m_audioSink->state();
  return state == QtAudio::ActiveState || state == QtAudio::IdleState;
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
  if (isSinkMeasurable()) {
    const auto capacity = m_audioSink->bufferSize();
    if (capacity > 0) {
      const auto used = capacity - m_audioSink->bytesFree();
      return static_cast<float>(used) / static_cast<float>(capacity);
    }
  }

  // TODO
  // With no sink there is no occupancy to report, and the cached 0 would read as room for another
  // frame every time it was asked — which is a game running as fast as the host can manage. A sink
  // that is merely between states still has its last real level to give
  if (!m_audioSink) {
    return -1.0f;
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

  spdlog::info("Reinitializing the audio output device.");

  // Only while the sink is still running: once it has stopped, this pointer is already dead and
  // touching it is what the rebuild exists to avoid
  if (m_audioDevice && isSinkWritable()) {
    m_audioDevice->close();
  }

  m_audioDevice = nullptr;
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

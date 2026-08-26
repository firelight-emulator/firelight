// TODO: NEEDS REVIEW
#include "performance_stats.hpp"

#include <algorithm>
#include <cmath>

namespace firelight::diagnostics {

PerformanceStats &PerformanceStats::instance() {
  static PerformanceStats stats;
  return stats;
}

void PerformanceStats::setCoreInfo(const int baseWidth, const int baseHeight, const int maxWidth, const int maxHeight,
                                   const double aspectRatio, const double coreFps) {
  std::lock_guard lock(m_mutex);
  m_snapshot.baseWidth = baseWidth;
  m_snapshot.baseHeight = baseHeight;
  m_snapshot.maxWidth = maxWidth;
  m_snapshot.maxHeight = maxHeight;
  m_snapshot.aspectRatio = aspectRatio;
  m_snapshot.coreFps = coreFps;
}

void PerformanceStats::setPacing(const std::string &mode, const double displayHz, const double audioRatio) {
  std::lock_guard lock(m_mutex);
  m_snapshot.pacingMode = mode;
  m_snapshot.displayHz = displayHz;
  m_snapshot.audioRatio = audioRatio;
}

void PerformanceStats::setVideo(const std::string &graphicsApi, const int renderWidth, const int renderHeight) {
  std::lock_guard lock(m_mutex);
  m_snapshot.graphicsApi = graphicsApi;
  m_snapshot.renderWidth = renderWidth;
  m_snapshot.renderHeight = renderHeight;
}

void PerformanceStats::setViewport(const int width, const int height) {
  std::lock_guard lock(m_mutex);
  m_snapshot.viewportWidth = width;
  m_snapshot.viewportHeight = height;
}

void PerformanceStats::recordFrame(const int64_t frameTimeNs, const int framesRun, const int framesRequested) {
  std::lock_guard lock(m_mutex);

  m_snapshot.framesRun += framesRun;
  m_snapshot.framesRequested += framesRequested;
  m_snapshot.framesLost = m_snapshot.framesRequested - m_snapshot.framesRun;

  if (frameTimeNs <= 0) {
    return;
  }

  const auto sample = static_cast<double>(frameTimeNs);

  if (!m_hasFrameTime) {
    m_frameTimeMeanNs = sample;
    m_frameTimeVarianceNs = 0.0;
    m_hasFrameTime = true;
  } else {
    // TODO
    // Mean and spread from the same exponential window, so the deviation describes the frames the
    // mean describes rather than the whole session
    const auto delta = sample - m_frameTimeMeanNs;
    m_frameTimeMeanNs += FRAME_SMOOTHING * delta;
    m_frameTimeVarianceNs = (1.0 - FRAME_SMOOTHING) * (m_frameTimeVarianceNs + FRAME_SMOOTHING * delta * delta);
  }

  // TODO
  // Counted rather than taken from the smoothed gap, because a pass that ran two frames is one gap
  // twice as long and averaging those reports a rate the game never ran at.
  //
  // Counted since the game started rather than over a window, because a window holds a whole number
  // of frames against a length that is not a whole number of them: one frame either way over a
  // second is more than a percent, which is the whole size of the difference being looked for here.
  // The total converges instead, and the deviation beside it is what shows movement
  // TODO
  // The opening seconds are a recompiler translating and a renderer building pipelines, and a total
  // that includes them spends minutes averaging them back out. They are skipped rather than weighted
  if (m_warmupFrames < WARMUP_FRAMES) {
    m_warmupFrames += framesRun;
  } else {
    m_totalElapsedNs += sample;
    m_totalFrames += framesRun;
  }

  if (m_totalFrames > 0 && m_totalElapsedNs > 0.0) {
    m_snapshot.frameRate = static_cast<double>(m_totalFrames) * 1e9 / m_totalElapsedNs;
  }

  // TODO
  // The smoothed gap rather than the total's reciprocal, which would only be the frame rate written
  // the other way round and could say nothing the frame rate had not already said
  m_snapshot.frameTimeMs = m_frameTimeMeanNs / 1e6;

  m_snapshot.frameTimeDeviationPercent =
      m_frameTimeMeanNs > 0.0 ? std::sqrt(m_frameTimeVarianceNs) / m_frameTimeMeanNs * 100.0 : 0.0;
}

void PerformanceStats::recordPresent(const int64_t gapNs) {
  if (gapNs <= 0) {
    return;
  }

  std::lock_guard lock(m_mutex);

  const auto sample = static_cast<double>(gapNs);

  if (!m_hasPresent) {
    m_presentMeanNs = sample;
    m_presentVarianceNs = 0.0;
    m_hasPresent = true;
  } else {
    const auto delta = sample - m_presentMeanNs;
    m_presentMeanNs += FRAME_SMOOTHING * delta;
    m_presentVarianceNs = (1.0 - FRAME_SMOOTHING) * (m_presentVarianceNs + FRAME_SMOOTHING * delta * delta);
  }

  m_snapshot.presentTimeMs = m_presentMeanNs / 1e6;
  m_snapshot.presentDeviationPercent =
      m_presentMeanNs > 0.0 ? std::sqrt(m_presentVarianceNs) / m_presentMeanNs * 100.0 : 0.0;
}

void PerformanceStats::recordWake(const int64_t marginNs, const int64_t overshootNs) {
  std::lock_guard lock(m_mutex);

  m_snapshot.spinMarginMs = static_cast<double>(marginNs) / 1e6;

  const auto sample = static_cast<double>(std::max<int64_t>(overshootNs, 0));
  m_wakeOvershootMeanNs += FRAME_SMOOTHING * (sample - m_wakeOvershootMeanNs);
  m_snapshot.wakeOvershootMeanMs = m_wakeOvershootMeanNs / 1e6;
  m_snapshot.wakeOvershootPeakMs = std::max(m_snapshot.wakeOvershootPeakMs, sample / 1e6);
}

void PerformanceStats::setAudioDevice(const std::string &name, const int capacityBytes, const double coreSampleRate) {
  std::lock_guard lock(m_mutex);
  m_snapshot.audioDevice = name;
  m_snapshot.bufferCapacityBytes = capacityBytes;
  m_snapshot.coreSampleRate = coreSampleRate;
}

void PerformanceStats::recordAudio(const double occupancy, const double correction, const int64_t samples) {
  std::lock_guard lock(m_mutex);

  m_snapshot.samplesDelivered += samples;
  m_snapshot.correctionPercent = correction * 100.0;

  const auto nearUnderrun = occupancy <= UNDERRUN_LEVEL ? 1.0 : 0.0;
  const auto nearBlocking = occupancy >= BLOCKING_LEVEL ? 1.0 : 0.0;

  if (!m_hasOccupancy) {
    m_occupancyMean = occupancy;
    m_occupancyVariance = 0.0;
    m_underrunShare = nearUnderrun;
    m_blockingShare = nearBlocking;
    m_hasOccupancy = true;
  } else {
    const auto delta = occupancy - m_occupancyMean;
    m_occupancyMean += SMOOTHING * delta;
    m_occupancyVariance = (1.0 - SMOOTHING) * (m_occupancyVariance + SMOOTHING * delta * delta);
    m_underrunShare += SMOOTHING * (nearUnderrun - m_underrunShare);
    m_blockingShare += SMOOTHING * (nearBlocking - m_blockingShare);
  }

  m_snapshot.bufferSaturationPercent = m_occupancyMean * 100.0;
  m_snapshot.bufferDeviationPercent =
      m_occupancyMean > 0.0 ? std::sqrt(m_occupancyVariance) / m_occupancyMean * 100.0 : 0.0;
  m_snapshot.closeToUnderrunPercent = m_underrunShare * 100.0;
  m_snapshot.closeToBlockingPercent = m_blockingShare * 100.0;
}

PerformanceSnapshot PerformanceStats::snapshot() const {
  std::lock_guard lock(m_mutex);
  return m_snapshot;
}

void PerformanceStats::reset() {
  std::lock_guard lock(m_mutex);
  m_snapshot.framesRun = 0;
  m_snapshot.framesRequested = 0;
  m_snapshot.framesLost = 0;
  m_snapshot.samplesDelivered = 0;
  m_frameTimeMeanNs = 0.0;
  m_frameTimeVarianceNs = 0.0;
  m_totalElapsedNs = 0.0;
  m_totalFrames = 0;
  m_warmupFrames = 0;
  m_occupancyMean = 0.0;
  m_occupancyVariance = 0.0;
  m_underrunShare = 0.0;
  m_blockingShare = 0.0;
  m_wakeOvershootMeanNs = 0.0;
  m_snapshot.wakeOvershootPeakMs = 0.0;
  m_presentMeanNs = 0.0;
  m_presentVarianceNs = 0.0;
  m_hasFrameTime = false;
  m_hasPresent = false;
  m_hasOccupancy = false;
}

} // namespace firelight::diagnostics

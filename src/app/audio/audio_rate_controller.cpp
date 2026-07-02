#include "audio_rate_controller.hpp"

#include <cmath>

void AudioRateController::reset() {
  for (int &v : m_usageBytes) {
    v = 0;
  }
  m_index = 0;
  m_populatedCount = 0;
  m_previousAvgFillRatio = -1.0;
}

int AudioRateController::computeCompensation(const int usedBytes,
                                             const int bufferCapacityBytes) {
  if (bufferCapacityBytes <= 0) {
    return 0;
  }

  // Rolling average of recent buffer occupancy.
  m_usageBytes[m_index] = usedBytes;
  m_index = (m_index + 1) % kWindowSize;
  if (m_populatedCount < kWindowSize) {
    m_populatedCount++;
  }

  long long sum = 0;
  for (int i = 0; i < m_populatedCount; ++i) {
    sum += m_usageBytes[i];
  }
  const double avgUsedBytes = static_cast<double>(sum) / m_populatedCount;
  const double avgFillRatio = avgUsedBytes / bufferCapacityBytes;

  constexpr double kTargetFillRatio = 0.5;
  const double targetFillBytes = bufferCapacityBytes * kTargetFillRatio;
  const double deviation = (avgUsedBytes - targetFillBytes) / targetFillBytes;

  // Skip adjusting while near the target or already trending back toward it —
  // only intervene once the average has stabilised (full window).
  bool adjust = true;
  if (m_previousAvgFillRatio >= 0.0 && m_populatedCount == kWindowSize) {
    const double currentError = avgFillRatio - kTargetFillRatio;
    const double previousError = m_previousAvgFillRatio - kTargetFillRatio;

    constexpr double kWithinTargetTolerance = 0.05; // ~45%-55% fill
    constexpr double kExtremeDeviation = 0.25;      // <25% or >75% fill

    const bool trendingWell = std::abs(currentError) < std::abs(previousError);
    const bool nearTarget = std::abs(currentError) <= kWithinTargetTolerance;
    const bool extreme = std::abs(currentError) > kExtremeDeviation;

    if (nearTarget || (trendingWell && !extreme)) {
      adjust = false;
    }
  }

  if (m_populatedCount == kWindowSize) {
    m_previousAvgFillRatio = avgFillRatio;
  }

  if (!adjust) {
    return 0;
  }

  // Buffer too full -> drop samples (negative); too empty -> add samples.
  if (deviation > 0.6) {
    return -5; // >80% full
  }
  if (deviation > 0.3) {
    return -4; // >65%
  }
  if (deviation > 0.1) {
    return -3; // >55%
  }
  if (deviation > -0.3) {
    return 0; // 35%-55% deadband
  }
  if (deviation > -0.6) {
    return 1; // 20%-35%
  }
  return 2; // <20%
}

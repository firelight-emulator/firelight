#include <firelight/audio/audio_rate_controller.hpp>

#include <cmath>

void AudioRateController::reset() {
  for (int &bytes: m_usageBytes) {
    bytes = 0;
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

  // Rolling average of recent buffer occupancy
  m_usageBytes[m_index] = usedBytes;
  m_index = (m_index + 1) % WINDOW_SIZE;

  if (m_populatedCount < WINDOW_SIZE) {
    m_populatedCount++;
  }

  long long sum = 0;
  for (int i = 0; i < m_populatedCount; ++i) {
    sum += m_usageBytes[i];
  }

  const double avgUsedBytes = static_cast<double>(sum) / m_populatedCount;
  const double avgFillRatio = avgUsedBytes / bufferCapacityBytes;

  constexpr double TARGET_FILL_RATIO = 0.5;
  const double targetFillBytes = bufferCapacityBytes * TARGET_FILL_RATIO;
  const double deviation = (avgUsedBytes - targetFillBytes) / targetFillBytes;

  // Skip adjusting while near the target or already trending back toward it
  bool adjust = true;
  if (m_previousAvgFillRatio >= 0.0 && m_populatedCount == WINDOW_SIZE) {
    const double currentError = avgFillRatio - TARGET_FILL_RATIO;
    const double previousError = m_previousAvgFillRatio - TARGET_FILL_RATIO;

    constexpr double WITHIN_TARGET_TOLERANCE = 0.05; // ~45%-55% fill
    constexpr double EXTREME_DEVIATION = 0.25; // <25% or >75% fill

    const bool trendingWell = std::abs(currentError) < std::abs(previousError);
    const bool nearTarget = std::abs(currentError) <= WITHIN_TARGET_TOLERANCE;
    const bool extreme = std::abs(currentError) > EXTREME_DEVIATION;

    if (nearTarget || (trendingWell && !extreme)) {
      adjust = false;
    }
  }

  if (m_populatedCount == WINDOW_SIZE) {
    m_previousAvgFillRatio = avgFillRatio;
  }

  if (!adjust) {
    return 0;
  }

  // Buffer too full, drop samples. Too empty, add samples
  if (deviation > 0.6) {
    return -5;
  }
  if (deviation > 0.3) {
    return -4;
  }
  if (deviation > 0.1) {
    return -3;
  }
  if (deviation > -0.3) {
    return 0;
  }
  if (deviation > -0.6) {
    return 1;
  }
  return 2;
}

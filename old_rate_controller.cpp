#include "old_rate_controller.hpp"

#include <cmath>

namespace {
// TODO
// How hard the rate is pulled, as a fraction of the output rate. These are the sample counts the
// controller used to return, divided by a typical callback's worth of output, so a buffer that was
// steady before stays steady now
constexpr double DRAIN_HARD = 0.00625;
constexpr double DRAIN_MEDIUM = 0.005;
constexpr double DRAIN_SOFT = 0.00375;
constexpr double FILL_SOFT = 0.00125;
constexpr double FILL_MEDIUM = 0.0025;
} // namespace

void OldAudioRateController::reset() {
  for (int &bytes : m_usageBytes) {
    bytes = 0;
  }

  m_index = 0;
  m_populatedCount = 0;
  m_previousAvgFillRatio = -1.0;
}

double OldAudioRateController::computeCompensation(const int usedBytes, const int bufferCapacityBytes, const int) {
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
    constexpr double EXTREME_DEVIATION = 0.25;       // <25% or >75% fill

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
    return 0.0;
  }

  // Buffer too full, shorten the audio. Too empty, stretch it. The further off target, the firmer
  // the answer
  if (deviation > 0.6) {
    return -DRAIN_HARD;
  }
  if (deviation > 0.3) {
    return -DRAIN_MEDIUM;
  }
  if (deviation > 0.1) {
    return -DRAIN_SOFT;
  }
  if (deviation > -0.3) {
    return 0.0;
  }
  if (deviation > -0.6) {
    return FILL_SOFT;
  }
  return FILL_MEDIUM;
}

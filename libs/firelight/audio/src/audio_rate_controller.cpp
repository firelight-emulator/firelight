// TODO: NEEDS REVIEW
#include <firelight/audio/audio_rate_controller.hpp>

#include <algorithm>
#include <cmath>

namespace {
constexpr int BYTES_PER_FRAME = 4; // interleaved stereo int16
} // namespace

void AudioRateController::reset() {
  m_averageUsedBytes = -1.0;
  m_standingCorrection = 0.0;
}

double AudioRateController::computeCompensation(const int usedBytes, const int bufferCapacityBytes,
                                                const int framesThisCall) {
  if (bufferCapacityBytes <= 0) {
    return 0.0;
  }

  const auto used = static_cast<double>(usedBytes);

  if (m_averageUsedBytes < 0.0) {
    m_averageUsedBytes = used;
  } else {
    // TODO
    // Weighted by how much audio this call carried rather than by the call itself, so the smoothing
    // spans the same amount of sound whether a core hands over one batch a frame or eight
    const auto windowBytes = SMOOTHING_BUFFERS * bufferCapacityBytes;
    const auto carriedBytes = static_cast<double>(std::max(framesThisCall, 0)) * BYTES_PER_FRAME;
    const auto weight = 1.0 - std::exp(-carriedBytes / windowBytes);

    m_averageUsedBytes += weight * (used - m_averageUsedBytes);
  }

  const auto half = bufferCapacityBytes / 2.0;
  const auto error = std::clamp((m_averageUsedBytes - half) / half, -1.0, 1.0);

  // TODO
  // Too full shortens the audio and too empty stretches it, in proportion to how far off it is
  const auto proportional = -MAX_CORRECTION * error;

  // TODO
  // Walks toward whatever correction the buffer needs on average, so that a mismatch is held by this
  // term rather than by the error term, and the buffer can sit on target while it is held. Measured
  // in the same audio the smoothing uses, so a core's batch size does not set the rate it moves at
  const auto carriedBuffers = static_cast<double>(std::max(framesThisCall, 0)) * BYTES_PER_FRAME / bufferCapacityBytes;
  const auto candidate = std::clamp(m_standingCorrection - MAX_CORRECTION / INTEGRAL_BUFFERS * error * carriedBuffers,
                                    -MAX_CORRECTION, MAX_CORRECTION);

  // TODO
  // Held where it is while the total is already asking for everything there is, so that a buffer the
  // correction cannot rescue does not keep winding this up and then take just as long to come back
  const auto total = proportional + candidate;
  if (std::abs(total) < MAX_CORRECTION || std::abs(candidate) < std::abs(m_standingCorrection)) {
    m_standingCorrection = candidate;
  }

  // TODO
  // A buffer sitting exactly on target is left alone without needing a dead zone to say so
  return std::clamp(proportional + m_standingCorrection, -MAX_CORRECTION, MAX_CORRECTION);
}

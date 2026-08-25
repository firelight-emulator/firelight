#include <firelight/audio/audio_rate_controller.hpp>

#include <algorithm>
#include <cmath>

namespace {
constexpr int BYTES_PER_FRAME = 4; // interleaved stereo int16
} // namespace

void AudioRateController::reset() { m_averageUsedBytes = -1.0; }

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
  // Too full shortens the audio and too empty stretches it, in proportion to how far off it is. The
  // error cannot leave -1..1, so the correction cannot leave MAX_CORRECTION, and a buffer sitting
  // exactly on target is left alone without needing a dead zone to say so
  return -MAX_CORRECTION * error;
}

// TODO: NEEDS REVIEW
#include "emulation_rate_controller.hpp"

#include <algorithm>
#include <cmath>

namespace firelight::emulation {

namespace {
/**
 * The longest gap that still counts as continuous play. A longer one means the loop was stopped
 * rather than late, and the time is not owed
 */
constexpr int64_t MAX_ELAPSED_NS = 250000000LL;

constexpr double NS_PER_SECOND = 1e9;

/**
 * How short of a whole frame still counts as due. A wait that returns a hair early — which real
 * timers do — should not cost a whole frame's delay. The shortfall is carried rather than forgiven,
 * so the rate is unaffected
 */
constexpr double DUE_EPSILON = 1e-4;
} // namespace

void EmulationRateController::configure(const PacingContext &context) {
  const auto modeChanged = context.mode != m_context.mode;
  m_context = context;
  resolveRates();

  // A mode change means the old cadence describes nothing; a rate change within a mode is a nudge
  // the phase can absorb
  if (modeChanged) {
    reset();
  }
}

void EmulationRateController::reset() {
  m_owedFrames = 0.0;
  m_lastTickNs = 0;
  m_refreshesSinceFrame = 0;
}

void EmulationRateController::resolveRates() {
  const auto contentFps = m_context.contentFps > 0.0 ? m_context.contentFps : 60.0;

  m_refreshesPerFrame = 0;
  m_effectiveFps = contentFps;
  m_audioRatio = 1.0;

  if (m_context.mode != SyncMode::Display || m_context.displayHz <= 0.0) {
    return;
  }

  // The whole number of refreshes closest to one frame's worth. A display that cannot hold a frame
  // for a whole number of refreshes cannot represent this content rate, and the mode gives up
  const auto refreshes = std::max(1, static_cast<int>(std::lround(m_context.displayHz / contentFps)));
  const auto candidateFps = m_context.displayHz / refreshes;

  if (std::abs(candidateFps - contentFps) / contentFps > DISPLAY_MATCH_TOLERANCE) {
    return;
  }

  m_refreshesPerFrame = refreshes;
  m_effectiveFps = candidateFps;
  // The game is running at the display's rate rather than its own, so the audio has to be stretched
  // by the same amount to stay in step with it
  m_audioRatio = candidateFps / contentFps;
}

void EmulationRateController::setAudioBufferLevel(const float level) { m_audioBufferLevel = level; }

int EmulationRateController::takeOwedFrames() {
  if (m_owedFrames + DUE_EPSILON < 1.0) {
    return 0;
  }

  const auto whole = static_cast<int>(m_owedFrames + DUE_EPSILON);

  // The debt is settled either way: anything past the cap is dropped rather than carried, so a
  // stall doesn't come back as a burst of fast-forward. Only the fraction — the phase — is kept
  m_owedFrames -= whole;

  return std::min(whole, MAX_CATCH_UP_FRAMES);
}

int EmulationRateController::framesDue(const int64_t nowNs) {
  // Display counts refreshes instead, so that frames land on them rather than near them
  if (m_context.mode == SyncMode::Display && m_refreshesPerFrame > 0) {
    return 0;
  }

  // Audio has no rate. A frame is due when there is room for the audio it will produce, which makes
  // the device's consumption the clock — and it stays right through anything a clock would drift
  // against: a device running slightly off its nominal rate, or a core whose own rate is misreported
  if (m_context.mode == SyncMode::Audio) {
    return m_audioBufferLevel >= 0.0 && m_audioBufferLevel < TARGET_BUFFER_LEVEL ? 1 : 0;
  }

  if (m_lastTickNs == 0) {
    m_lastTickNs = nowNs;
    // The first frame runs immediately; there is no interval to have waited out yet
    return 1;
  }

  const auto elapsedNs = nowNs - m_lastTickNs;

  if (elapsedNs <= 0) {
    return 0;
  }

  m_lastTickNs = nowNs;

  if (elapsedNs > MAX_ELAPSED_NS) {
    // Nothing was owed across a gap this long — the loop was not running, and pretending otherwise
    // would run a burst of frames for time the player did not experience
    return 1;
  }

  m_owedFrames += static_cast<double>(elapsedNs) / NS_PER_SECOND * m_effectiveFps;

  return takeOwedFrames();
}

int EmulationRateController::framesDueOnPresent() {
  if (m_context.mode != SyncMode::Display) {
    return 0;
  }

  if (m_refreshesPerFrame <= 0) {
    return 0;
  }

  m_refreshesSinceFrame++;

  if (m_refreshesSinceFrame < m_refreshesPerFrame) {
    return 0;
  }

  m_refreshesSinceFrame = 0;

  return 1;
}

int64_t EmulationRateController::getNextDeadlineNs() const {
  // Neither of these runs on a clock: Display waits for a refresh, Audio for room in the sink
  if (m_context.mode == SyncMode::Audio) {
    return 0;
  }

  if (m_context.mode == SyncMode::Display && m_refreshesPerFrame > 0) {
    return 0;
  }

  if (m_lastTickNs == 0 || m_effectiveFps <= 0.0) {
    return 0;
  }

  const auto remainingFrames = std::max(0.0, 1.0 - m_owedFrames);

  return m_lastTickNs + static_cast<int64_t>(remainingFrames / m_effectiveFps * NS_PER_SECOND);
}

} // namespace firelight::emulation

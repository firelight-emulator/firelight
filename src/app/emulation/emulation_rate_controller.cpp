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
  const auto previousMode = m_resolvedMode;
  const auto previousFps = m_effectiveFps;
  const auto previousRefreshes = m_refreshesPerFrame;

  m_context = context;
  resolveRates();

  // Anything that changes the cadence leaves the phase built for the old one describing nothing —
  // half of a two-refresh frame is not half of a four-refresh one
  if (m_resolvedMode != previousMode || m_effectiveFps != previousFps || m_refreshesPerFrame != previousRefreshes) {
    reset();
  }
}

void EmulationRateController::reset() {
  m_owedFrames = 0.0;
  m_lastTickNs = 0;
  m_refreshesSinceFrame = 0;
}

double EmulationRateController::displayLockedFps(const double contentFps) const {
  if (m_context.displayHz <= 0.0 || contentFps <= 0.0) {
    return 0.0;
  }

  // The whole number of refreshes closest to one frame's worth. A display that cannot hold a frame
  // for a whole number of refreshes cannot represent this content rate
  const auto refreshes = std::max(1, static_cast<int>(std::lround(m_context.displayHz / contentFps)));
  const auto candidateFps = m_context.displayHz / refreshes;

  if (std::abs(candidateFps - contentFps) / contentFps > DISPLAY_MATCH_TOLERANCE) {
    return 0.0;
  }

  return candidateFps;
}

void EmulationRateController::resolveRates() {
  const auto contentFps = m_context.contentFps > 0.0 ? m_context.contentFps : 60.0;

  m_resolvedMode = m_context.mode;
  m_followingDisplay = false;
  m_refreshesPerFrame = 0;
  m_effectiveFps = contentFps;
  m_audioRatio = 1.0;

  // Running the game at a rate it didn't ask for means stretching its audio by the same amount to
  // stay in step with it
  const auto runClockAt = [this, contentFps](const double fps) {
    m_resolvedMode = SyncMode::Fixed;
    m_followingDisplay = true;
    m_effectiveFps = fps;
    m_audioRatio = fps / contentFps;
  };

  const auto countRefreshesFor = [this, contentFps](const double fps) {
    m_resolvedMode = SyncMode::Display;
    m_followingDisplay = true;
    m_refreshesPerFrame = static_cast<int>(std::lround(m_context.displayHz / fps));
    m_effectiveFps = fps;
    m_audioRatio = fps / contentFps;
  };

  // Every other mode names its own pacing outright and is already resolved above
  if (m_context.mode != SyncMode::Auto && m_context.mode != SyncMode::Display) {
    return;
  }

  // TODO
  // Never the sink, for either of them. Pacing off audio occupancy cannot put frames out evenly: the
  // device only reports what it has taken in whole periods — 10.67 ms, measured — so the gate can
  // only fire on those boundaries and frames leave 10.7 or 21.3 ms apart, never the 16.67 they are
  // due. Audio is made to fit a clock instead, by rate control, which is what every other emulator
  // settled on
  const auto lockedFps = displayLockedFps(contentFps);

  if (lockedFps <= 0.0) {
    // No whole number of refreshes shows this content rate, so there is nothing to follow and the
    // rate the content asked for is the honest answer
    m_resolvedMode = SyncMode::Fixed;
    return;
  }

  if (!m_context.presentationLocked) {
    // The rate is right, but presents that don't wait for the display cannot be trusted to mark
    // refreshes — counting them would run the game at whatever rate they happen to arrive at. A
    // clock at the rate the display can hold is the closest thing available
    runClockAt(lockedFps);
    return;
  }

  // Presentation waits for the refresh, so counting refreshes puts each frame on an exact number of
  // them — nearer than a clock can get, with no beat left to drift
  countRefreshesFor(lockedFps);
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
  if (m_resolvedMode == SyncMode::Display && m_refreshesPerFrame > 0) {
    return 0;
  }

  // Audio has no rate. A frame is due when there is room for the audio it will produce, which makes
  // the device's consumption the clock — and it stays right through anything a clock would drift
  // against: a device running slightly off its nominal rate, or a core whose own rate is misreported
  // TODO
  // A level below zero is a sink that cannot be read — one being rebuilt, or one that was lost.
  // Falling through to the clock keeps frames running, and a frame is the only thing that calls into
  // the sink and so the only thing that can rebuild it. Gating on a level that cannot be read would
  // stop the emulation for good
  if (m_resolvedMode == SyncMode::Audio && m_audioBufferLevel >= 0.0) {
    return m_audioBufferLevel < TARGET_BUFFER_LEVEL ? 1 : 0;
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
  if (m_resolvedMode != SyncMode::Display) {
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
  if (m_resolvedMode == SyncMode::Audio) {
    return 0;
  }

  if (m_resolvedMode == SyncMode::Display && m_refreshesPerFrame > 0) {
    return 0;
  }

  if (m_lastTickNs == 0 || m_effectiveFps <= 0.0) {
    return 0;
  }

  const auto remainingFrames = std::max(0.0, 1.0 - m_owedFrames);

  return m_lastTickNs + static_cast<int64_t>(remainingFrames / m_effectiveFps * NS_PER_SECOND);
}

} // namespace firelight::emulation

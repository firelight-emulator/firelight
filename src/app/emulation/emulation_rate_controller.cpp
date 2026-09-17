// TODO: NEEDS REVIEW
#include "emulation_rate_controller.hpp"

#include <algorithm>
#include <cmath>

namespace firelight::emulation {

namespace {
constexpr double NS_PER_SECOND = 1e9;

/**
 * How short of the deadline still counts as due. A wait that returns a hair early — which real
 * timers do — should not cost a whole frame's delay
 */
constexpr int64_t DUE_EPSILON_NS = 100000;

/**
 * How fast the wake peak forgets a late wake, as a fraction per frame
 */
constexpr int64_t WAKE_PEAK_DECAY = 64;
} // namespace

void EmulationRateController::configure(const PacingContext &context) {
  const auto previousMode = m_resolvedMode;
  const auto previousFps = m_effectiveFps;
  const auto previousRefreshes = m_refreshesPerFrame;
  const auto previousHeld = m_heldRefreshes;
  const auto previousDisplayPeriod = m_displayPeriodNs;
  const auto previousPeriodNs = m_periodNs;

  m_context = context;
  m_ratesFromPeriodNs = 0;
  resolveRates(m_context.displayHz);

  // Anything that changes the cadence leaves the phase built for the old one describing nothing
  if (m_resolvedMode != previousMode || m_effectiveFps != previousFps || m_refreshesPerFrame != previousRefreshes ||
      m_heldRefreshes != previousHeld) {
    carryTheScheduleOver(previousPeriodNs);
  }

  if (m_displayPeriodNs != previousDisplayPeriod) {
    m_phase.configure(m_displayPeriodNs);
  }
}

void EmulationRateController::reset() {
  m_anchorNs = 0;
  m_slipDebtNs = 0;
  m_wasHolding = false;
  m_snapPending = false;
}

// TODO
// The cadence changed under a game that is still running, so the new one carries on from where the
// old one had got to: the frame already due stays due, moved by the difference between the periods,
// and what the clock owes is owed at either rate. Only the phase starts over
void EmulationRateController::carryTheScheduleOver(const int64_t previousPeriodNs) {
  if (m_anchorNs != 0 && previousPeriodNs > 0) {
    m_anchorNs += m_periodNs - previousPeriodNs;
  }

  m_wasHolding = false;
  m_snapPending = false;
}

bool EmulationRateController::isPhaseLocked() const {
  if (!m_context.presentationLocked || m_displayPeriodNs <= 0) {
    return false;
  }

  return m_resolvedMode == SyncMode::Display || (m_resolvedMode == SyncMode::Fixed && m_heldRefreshes > 0);
}

bool EmulationRateController::isHolding() const {
  return isPhaseLocked() && m_phase.isLocked() && m_phase.getPeriodNs() > 0;
}

void EmulationRateController::resolveRates(const double displayHz) {
  const auto contentFps = m_context.contentFps > 0.0 ? m_context.contentFps : 60.0;

  m_resolvedMode = m_context.mode;
  m_refreshesPerFrame = 0;
  m_heldRefreshes = 0;
  m_effectiveFps = contentFps;
  m_audioRatio = 1.0;
  m_displayPeriodNs = displayHz > 0.0 ? static_cast<int64_t>(NS_PER_SECOND / displayHz) : 0;

  const auto wantsDisplay = m_context.mode == SyncMode::Auto || m_context.mode == SyncMode::Display;

  if (wantsDisplay) {
    if (displayHz <= 0.0) {
      m_resolvedMode = SyncMode::Fixed;
    } else {
      // TODO
      // The whole number of refreshes nearest one frame, and the rate that division gives. Display
      // takes it whatever it costs in speed; Auto only when the cost cannot be heard
      const auto refreshes = std::max(1, static_cast<int>(std::lround(displayHz / contentFps)));
      const auto dividedFps = displayHz / refreshes;
      const auto error = std::abs(dividedFps - contentFps) / contentFps;

      if (m_context.mode == SyncMode::Auto && error > DISPLAY_MATCH_TOLERANCE) {
        m_resolvedMode = SyncMode::Fixed;
      } else {
        m_resolvedMode = SyncMode::Display;
        m_refreshesPerFrame = refreshes;
        m_effectiveFps = dividedFps;
        m_audioRatio = dividedFps / contentFps;
      }
    }
  }

  // TODO
  // A Fixed clock whose rate is a whole division of the display's steps on its grid, by however many
  // refreshes come nearest to one frame
  if (m_resolvedMode == SyncMode::Fixed && m_context.shouldHoldFixedClock && m_context.presentationLocked &&
      displayHz > 0.0) {
    const auto refreshes = std::max(1, static_cast<int>(std::lround(displayHz / contentFps)));
    const auto dividedFps = displayHz / refreshes;

    if (m_context.isHoldForced || std::abs(dividedFps - contentFps) / contentFps <= HOLD_TOLERANCE) {
      m_heldRefreshes = refreshes;
    }
  }

  m_periodNs = m_effectiveFps > 0.0 ? static_cast<int64_t>(NS_PER_SECOND / m_effectiveFps) : 0;
}

// TODO
// The rate a display reports is not always the rate it runs at, and every decision here is about the
// rate it runs at: which mode fits, whether the clock is held, and what the audio has to be bent by
void EmulationRateController::adoptMeasuredRefresh() {
  if (!m_context.presentationLocked || !m_phase.isLocked()) {
    return;
  }

  const auto measuredNs = m_phase.getPeriodNs();

  if (measuredNs <= 0) {
    return;
  }

  const auto builtOnNs = m_ratesFromPeriodNs > 0 ? m_ratesFromPeriodNs : m_displayPeriodNs;

  if (builtOnNs > 0 &&
      std::abs(static_cast<double>(measuredNs - builtOnNs)) / static_cast<double>(builtOnNs) < MEASURED_RATE_STEP) {
    return;
  }

  const auto previousPeriodNs = m_periodNs;
  m_ratesFromPeriodNs = measuredNs;
  resolveRates(NS_PER_SECOND / static_cast<double>(measuredNs));
  carryTheScheduleOver(previousPeriodNs);
}

void EmulationRateController::setAudioBufferLevel(const float level) { m_audioBufferLevel = level; }

int EmulationRateController::framesDue(const int64_t nowNs) {
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

  if (m_periodNs <= 0) {
    return 0;
  }

  adoptMeasuredRefresh();
  const auto holding = isHolding();

  // TODO
  // What was owed is owed under either schedule, so only the phase is rebuilt
  if (holding != m_wasHolding) {
    m_wasHolding = holding;
    m_snapPending = holding;
  }

  if (m_anchorNs == 0) {
    // The first frame runs immediately; the next is due one period on, or at the first target
    // instant far enough ahead when the grid is known
    m_anchorNs = nowNs + m_periodNs;

    if (holding) {
      const auto refreshNs = m_phase.getPeriodNs();
      auto instantNs = m_phase.getGridNs(nowNs) - std::llround((1.0 - m_phaseTarget) * static_cast<double>(refreshNs));

      while (instantNs < nowNs + refreshNs / 2) {
        instantNs += refreshNs;
      }

      m_anchorNs = instantNs;
      m_snapPending = false;
    }

    m_slipDebtNs = 0;
    m_phaseLockedSeries.record(m_phase.isLocked() ? 1.0 : 0.0);
    m_refreshPeriodSeries.record(static_cast<double>(m_phase.getPeriodNs()) / 1e6);
    return 1;
  }

  if (nowNs + DUE_EPSILON_NS < m_anchorNs) {
    return 0;
  }

  const auto lateNs = nowNs - m_anchorNs;

  // TODO
  // Late by less than two periods catches up with one frame run straight after this one, which the
  // pass shows the newer of, the same as a slip. Later than that the frames that would have filled
  // the gap are dropped, not run in a burst, and the cadence restarts from now
  if (lateNs > 2 * m_periodNs) {
    m_anchorNs = nowNs + m_periodNs;
    m_timeDroppedMarker.mark();
    m_snapPending = holding;
  } else if (holding) {
    stepOnTheGrid();
  } else {
    m_anchorNs += m_periodNs;
  }

  // TODO
  // Put straight onto the target rather than nudged there over frames, so a game that has just
  // loaded, unpaused or come under the hold does not spend seconds crawling. Landing on the target
  // of the refresh the frame would reach anyway crosses no present in either direction
  if (m_snapPending && holding) {
    const auto targetNs = targetInstantFor(m_anchorNs);
    const auto beforeNs = m_anchorNs;

    if (std::abs(targetNs - m_anchorNs) >= std::llround(SNAP_TOLERANCE * static_cast<double>(m_phase.getPeriodNs()))) {
      m_anchorNs = targetNs;
    }

    // TODO
    // The jump is time the content has not been run for, so the tally carries it and a slip pays it
    m_slipDebtNs += m_anchorNs - beforeNs;
    m_snapPending = false;
  }

  m_slipDebtSeries.record(static_cast<double>(m_slipDebtNs) / 1e6);

  m_phaseLockedSeries.record(m_phase.isLocked() ? 1.0 : 0.0);
  m_refreshPeriodSeries.record(static_cast<double>(m_phase.getPeriodNs()) / 1e6);
  return 1;
}

void EmulationRateController::stepOnTheGrid() {
  const auto refreshNs = m_phase.getPeriodNs();
  const auto refreshes = m_resolvedMode == SyncMode::Display ? std::max(m_refreshesPerFrame, 1) : m_heldRefreshes;
  auto stepNs = refreshes * refreshNs;
  auto slipped = false;

  // TODO
  // A frame owed is paid by not moving the anchor, so the next tick runs at once and the pass shows
  // only the later of the two; a frame run ahead is paid by moving it a refresh further, so one
  // refresh shows nothing new. The tally is against how far the anchor really moved, nudge and
  // all, so the rate over a beat is the content's exactly whatever the grid is measured as
  if (m_resolvedMode == SyncMode::Fixed) {
    if (m_slipDebtNs >= refreshNs) {
      stepNs -= refreshNs;
      slipped = true;
    } else if (m_slipDebtNs <= -refreshNs) {
      stepNs += refreshNs;
      slipped = true;
    }
  }

  const auto beforeNs = m_anchorNs;
  m_anchorNs += stepNs;
  correctPhase();

  if (m_resolvedMode == SyncMode::Fixed) {
    m_slipDebtNs += (m_anchorNs - beforeNs) - m_periodNs;
  }

  if (slipped) {
    m_slipMarker.mark();
  }
}

int64_t EmulationRateController::targetInstantFor(const int64_t anchorNs) const {
  const auto refreshNs = m_phase.getPeriodNs();
  const auto readyNs = anchorNs + m_slowestFrameNs + m_passTimeNs;
  auto gridNs = m_phase.getGridNs(readyNs);

  if (gridNs < readyNs) {
    gridNs += refreshNs;
  }

  return gridNs - std::llround((1.0 - m_phaseTarget) * static_cast<double>(refreshNs));
}

void EmulationRateController::notePresent(const int64_t nowNs) { m_phase.notePresent(nowNs); }

void EmulationRateController::noteFrameDuration(const int64_t durationNs) {
  if (m_frameTimeNs == 0) {
    m_frameTimeNs = durationNs;
    m_slowestFrameNs = durationNs;
    m_quickestFrameNs = durationNs;
    updatePhaseTarget();
    return;
  }

  // TODO
  // A frame over twice the smoothed time is a hitch, not the slow frame: it moves the smoothed
  // time, so a lasting change is still followed, but not what the target leaves room for
  if (durationNs > 2 * m_frameTimeNs) {
    m_frameTimeNs = (m_frameTimeNs * 7 + durationNs) / 8;
    updatePhaseTarget();
    return;
  }

  if (durationNs >= m_frameTimeNs) {
    m_slowSpreadNs = (m_slowSpreadNs * 7 + std::abs(durationNs - m_slowestFrameNs)) / 8;
    m_slowestFrameNs = (m_slowestFrameNs * 7 + durationNs) / 8;
  } else {
    m_quickestFrameNs = (m_quickestFrameNs * 7 + durationNs) / 8;
  }

  m_frameTimeNs = (m_frameTimeNs * 7 + durationNs) / 8;
  updatePhaseTarget();
}

void EmulationRateController::notePassDuration(const int64_t durationNs) {
  m_passTimeNs = m_passTimeNs == 0 ? durationNs : (m_passTimeNs * 7 + durationNs) / 8;
  updatePhaseTarget();
}

void EmulationRateController::noteWakeLateness(const int64_t lateNs) {
  const auto boundedNs = std::clamp<int64_t>(lateNs, 0, WAKE_PEAK_CAP_NS);
  m_wakePeakNs = std::max(boundedNs, m_wakePeakNs - m_wakePeakNs / WAKE_PEAK_DECAY);
  updatePhaseTarget();
}

// TODO
// The asserted margin is capped by the refresh it has to fit inside, so a short refresh is not spent
// entirely on a margin nothing measured. What the wake really costs is never capped
int64_t EmulationRateController::getWakeMarginNs() const {
  const auto periodNs = m_phase.getPeriodNs() > 0 ? m_phase.getPeriodNs() : m_displayPeriodNs;
  const auto assertedNs = periodNs > 0 ? std::min(WAKE_MARGIN_NS, periodNs / MARGIN_SHARE_OF_REFRESH) : WAKE_MARGIN_NS;

  return std::max(assertedNs, m_wakePeakNs);
}

void EmulationRateController::setPhaseTarget(const double target) {
  m_pinnedPhaseTarget = target;
  updatePhaseTarget();
}

void EmulationRateController::updatePhaseTarget() {
  if (m_pinnedPhaseTarget >= 0.0) {
    m_phaseTarget = std::clamp(m_pinnedPhaseTarget, EARLIEST_PHASE_TARGET, MAX_PHASE_TARGET);
    return;
  }

  const auto periodNs = m_phase.getPeriodNs() > 0 ? m_phase.getPeriodNs() : m_displayPeriodNs;

  if (m_frameTimeNs == 0 || periodNs <= 0) {
    m_phaseTarget = PHASE_TARGET;
    return;
  }

  // TODO
  // The frame is asked for so that its picture is ready, even on its slowest frames and however
  // far they stray, the pass has put it up, and the wake was late by as much as it lately has
  // been, all just before the next refresh. When that would put the quickest picture before the
  // pass of the refresh the frame starts in, the frame is asked for no earlier than lands it past
  // that pass, so every frame is shown at the refresh after
  const auto period = static_cast<double>(periodNs);
  const auto spentNs = static_cast<double>(m_slowestFrameNs + m_passTimeNs + getWakeMarginNs()) +
                       SLOW_SPREAD_ALLOWANCE * static_cast<double>(m_slowSpreadNs);
  // TODO
  // A frame the step gives several refreshes to may be aimed back through them
  const auto steps = std::max(1, m_resolvedMode == SyncMode::Display ? m_refreshesPerFrame : m_heldRefreshes);
  const auto earliestOfAll = EARLIEST_PHASE_TARGET - static_cast<double>(steps - 1);
  const auto earliest =
      std::max(earliestOfAll, static_cast<double>(m_passTimeNs + getWakeMarginNs() - m_quickestFrameNs) / period);
  m_phaseTarget = std::clamp(1.0 - spentNs / period, std::min(earliest, MAX_PHASE_TARGET), MAX_PHASE_TARGET);
}

void EmulationRateController::correctPhase() {
  if (!isHolding()) {
    return;
  }

  const auto periodNs = static_cast<double>(m_phase.getPeriodNs());

  // TODO
  // Where the anchor sits inside the refresh that precedes it, 0..1, against where it should sit
  auto offset = static_cast<double>(m_anchorNs - m_phase.getGridNs(m_anchorNs)) / periodNs;

  if (offset < 0.0) {
    offset += 1.0;
  }

  // TODO
  // Never across a present: pulling the anchor back across one puts two frames in a refresh, and
  // pushing it forward across one shows a frame twice. A target at or before the present is the
  // end of the refresh before, and an anchor already there is measured from that same present
  auto delta = m_phaseTarget - offset;

  if (m_phaseTarget <= 0.0 && offset >= 0.5) {
    delta = m_phaseTarget - (offset - 1.0);
  }

  const auto nudgeNs = std::clamp(delta * CORRECTION_GAIN, -MAX_NUDGE, MAX_NUDGE) * periodNs;
  m_anchorNs += static_cast<int64_t>(std::llround(nudgeNs));
}

int64_t EmulationRateController::getNextDeadlineNs() const {
  if (m_resolvedMode == SyncMode::Audio) {
    return 0;
  }

  return m_anchorNs;
}

} // namespace firelight::emulation

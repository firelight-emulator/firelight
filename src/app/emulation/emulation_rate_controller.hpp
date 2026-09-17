// TODO: NEEDS REVIEW
#pragma once

#include "phase_estimator.hpp"

#include <firelight/monitoring/monitor.hpp>

#include <cstdint>

namespace firelight::emulation {

/**
 * How gameplay is paced.
 */
enum class SyncMode {
  /** Whichever of the modes below suits the display, the content rate and the audio device best */
  Auto,
  /** Frames run whenever the sink has room for more audio; the device is the clock, and no rate is
      involved at all */
  Audio,
  /** A wall clock at an explicit rate. Where presentation waits for the display and the rate is
      close enough, the clock is held to the display's phase and the difference in rate is paid as
      one frame doubled or repeated per beat */
  Fixed,
  /** A wall clock at a whole division of the display's rate, held to the display's phase where
      presentation waits for it, with the audio bent by the difference */
  Display
};

/**
 * What the controller is told about the world. Rates are in Hz.
 */
struct PacingContext {
  SyncMode mode = SyncMode::Fixed;
  /** The rate the game is meant to run at, whatever decided it */
  double contentFps = 60.0;
  /** The display's refresh rate, or 0 when it isn't known */
  double displayHz = 0.0;
  /** Whether presentation waits for the display's refresh. When it doesn't, a present says nothing
      about when a refresh happened, and the display's phase cannot be followed */
  bool presentationLocked = false;
  // TODO
  /** Whether a Fixed clock may be held to the display's grid. When it may not, it steps by its own
      period whatever the display does */
  bool shouldHoldFixedClock = true;
  // TODO
  /** Whether a Fixed clock is held however far its rate sits from a whole division of the display's */
  bool isHoldForced = false;
};

/**
 * Decides when the emulator should advance a frame, and what the audio has to be resampled by for
 * the result to play at the right speed.
 *
 * Every clock-driven mode runs on one schedule: an anchor that advances by one period per frame. A
 * frame that is late by less than a period runs now and the anchor keeps its grid; one later than
 * that re-anchors, so debt is dropped rather than repaid. While the presents describe a grid and
 * the mode holds to it, the anchor steps by whole refreshes instead and is nudged toward a target
 * phase of the refresh. Display mode bends the audio by what that costs in rate; Fixed mode keeps
 * a tally of what it owes the content's own rate and pays it as one frame per beat.
 *
 * It owns the policy and nothing else: no threads, no timers, no renderer, no Qt, so the same
 * decision can be tested against a synthetic sequence of timestamps.
 */
class EmulationRateController {
public:
  /**
   * How full the sink has to get before Audio mode stops asking for frames
   */
  static constexpr double TARGET_BUFFER_LEVEL = 0.5;

  /**
   * How close a whole number of refreshes has to land to the content rate for Auto to take the
   * display's rate. The game then runs at the display's rate with its audio stretched by the
   * difference, so the bound is what that stretch is worth before it is heard: 1% is about 17 cents.
   * Every rate real content reports lands inside 0.5% of a whole division — 59.94, 60.0988 and
   * 59.7275 against 60 Hz — so nothing that should match is turned away. Display mode named outright
   * is not gated by this
   */
  static constexpr double DISPLAY_MATCH_TOLERANCE = 0.01;

  // TODO
  /**
   * How far a Fixed clock's rate may sit from a whole division of the display's and still be held to
   * its grid. Further out, a frame moves further against the grid each frame than the timing varies,
   * so which refresh it reaches never depends on the variation
   */
  static constexpr double HOLD_TOLERANCE = 0.01;

  /**
   * Where in the refresh interval a frame is asked for, as a fraction after the present that starts
   * it, while the phase is held and nothing has been measured yet
   */
  static constexpr double PHASE_TARGET = 0.75;

  /**
   * The least the measured target leaves between a frame's picture and the pass that takes it, in
   * ns: the loop waking late, the pass starting, and how much either of them varies. The wake
   * lateness the loop reports widens it
   */
  static constexpr int64_t WAKE_MARGIN_NS = 4000000;

  // TODO
  /**
   * The most of a refresh the asserted margin may take, as a divisor: a 4 ms margin is a quarter of a
   * 60 Hz refresh but most of a 240 Hz one, and a margin that fills the refresh leaves nowhere to aim
   */
  static constexpr int64_t MARGIN_SHARE_OF_REFRESH = 3;

  // TODO
  /**
   * The most a late wake widens the margin by, so one long stall does not push the frame to the
   * start of the refresh for seconds
   */
  static constexpr int64_t WAKE_PEAK_CAP_NS = 8000000;

  // TODO
  /**
   * How far the measured target may go, in refreshes before the one the frame is aimed at. A frame
   * that cannot fit before the next present with its slowest and its spread allowed for is aimed so
   * that even its quickest lands past the pass that would take it, and every frame is shown the same
   * refresh on rather than half of them either side. A step of several refreshes has that many to
   * reach back through
   */
  static constexpr double EARLIEST_PHASE_TARGET = -0.5;
  static constexpr double MAX_PHASE_TARGET = 0.95;

  // TODO
  /**
   * How many spreads of the slow frame the target leaves room for
   */
  static constexpr double SLOW_SPREAD_ALLOWANCE = 2.0;

  /**
   * How much of the distance to the target phase one frame closes
   */
  static constexpr double CORRECTION_GAIN = 0.1;

  /**
   * The most one frame may move the anchor, as a fraction of a period. Half a percent of speed for
   * one frame, under what the tolerance above lets be heard
   */
  static constexpr double MAX_NUDGE = 0.005;

  // TODO
  /**
   * How far from the target, as a fraction of a refresh, an anchor that has just come under the
   * hold is put straight onto it rather than nudged there over frames
   */
  static constexpr double SNAP_TOLERANCE = 0.1;

  // TODO
  /**
   * How far the measured refresh must sit from the rate the rates were built on before they are
   * built again on the measurement, as a fraction
   */
  static constexpr double MEASURED_RATE_STEP = 0.0002;

  /**
   * Applies a new mode, content rate or display rate, keeping the phase that is still meaningful
   */
  void configure(const PacingContext &context);

  /**
   * @return What the controller was last configured with
   */
  [[nodiscard]] const PacingContext &getContext() const { return m_context; }

  /**
   * @return The mode actually in force — what Auto settled on, and otherwise the configured mode
   */
  [[nodiscard]] SyncMode getResolvedMode() const { return m_resolvedMode; }

  /**
   * @return Whether the rate in force came from the display rather than from the content
   */
  [[nodiscard]] bool isFollowingTheDisplay() const { return m_resolvedMode == SyncMode::Display; }

  /**
   * @return Whether the anchor is held to the display's phase, which needs the display's rate,
   *   presentation that waits for its refresh, and a mode that holds
   */
  [[nodiscard]] bool isPhaseLocked() const;

  /**
   * Drops accumulated timing state, so the next call starts a fresh cadence. For a game being
   * loaded, unpaused, or seeked, where the gap since the last frame means nothing
   */
  void reset();

  /**
   * Whether a frame is owed as of nowNs, consuming it. Never more than one, and the anchor moved
   * on exactly when one is.
   *
   * Audio answers from the sink rather than the clock: nowNs means nothing to it
   */
  [[nodiscard]] int framesDue(int64_t nowNs);

  /**
   * Feeds a frame reaching the display, for the phase the anchor is held to
   */
  void notePresent(int64_t nowNs);

  /**
   * How long the last frame took from its tick to its picture being ready, which the phase target
   * leaves room for
   */
  void noteFrameDuration(int64_t durationNs);

  /**
   * How long the last pass took to put a picture on the target
   */
  void notePassDuration(int64_t durationNs);

  /**
   * How late past its deadline the loop woke, which the phase target leaves room for
   */
  void noteWakeLateness(int64_t lateNs);

  /**
   * Pins the phase target to target, or lets it follow the measurements again when negative
   */
  void setPhaseTarget(double target);

  /**
   * @return Where in the refresh a frame is currently asked for, 0 to 1
   */
  [[nodiscard]] double getPhaseTarget() const { return m_phaseTarget; }

  [[nodiscard]] int64_t getFrameTimeNs() const { return m_frameTimeNs; }

  [[nodiscard]] int64_t getSlowestFrameNs() const { return m_slowestFrameNs; }

  [[nodiscard]] int64_t getQuickestFrameNs() const { return m_quickestFrameNs; }

  [[nodiscard]] int64_t getSlowSpreadNs() const { return m_slowSpreadNs; }

  /**
   * @return What the measured target currently leaves for a late wake and the pass starting, in ns
   */
  [[nodiscard]] int64_t getWakeMarginNs() const;

  /**
   * @return How much of the content's time the held clock has not yet run, in ns. Positive when a
   *   frame is owed, negative when one has been run ahead
   */
  [[nodiscard]] int64_t getSlipDebtNs() const { return m_slipDebtNs; }

  /**
   * When the next frame is due, for a caller that wants to wait rather than poll. 0 for Audio and
   * before the first frame
   */
  [[nodiscard]] int64_t getNextDeadlineNs() const;

  /**
   * Tells Audio mode how full the sink buffer is, 0..1, or a negative value when it can't be read.
   * Room in the buffer is the only thing that makes a frame due in that mode
   */
  void setAudioBufferLevel(float level);

  /**
   * @return What the emulator's audio has to be resampled by to play at the right speed. 1.0 unless
   *   Display mode is running the game at a rate the content didn't ask for
   */
  [[nodiscard]] double getAudioRatio() const { return m_audioRatio; }

  /**
   * @return The rate frames are actually being produced at, which is the content rate except where
   *   Display mode rounded it to the refresh
   */
  [[nodiscard]] double getEffectiveFps() const { return m_effectiveFps; }

  /**
   * @return How many refreshes Display mode holds each frame for, or 0 when it isn't in use
   */
  [[nodiscard]] int getRefreshesPerFrame() const { return m_refreshesPerFrame; }

  /**
   * @return How many refreshes a held Fixed clock steps by, or 0 when it is not held
   */
  [[nodiscard]] int getHeldRefreshes() const { return m_heldRefreshes; }

  /**
   * @return The grid the phase is held to
   */
  [[nodiscard]] const PhaseEstimator &getPhase() const { return m_phase; }

private:
  /**
   * Settles which mode is in force, the rate frames will be produced at, the audio ratio that
   * follows from it, and how many refreshes a held clock steps by, from the refresh rate given
   */
  void resolveRates(double displayHz);

  /**
   * Moves the schedule onto a cadence that has just changed, keeping when the next frame is due and
   * what the clock owes
   */
  void carryTheScheduleOver(int64_t previousPeriodNs);

  // TODO
  /**
   * Rebuilds those from the refresh the presents measure, once they have settled on one that differs
   * from what the rates were built on
   */
  void adoptMeasuredRefresh();

  /**
   * Whether the anchor is stepping on the grid right now: held, and the presents agree on one
   */
  [[nodiscard]] bool isHolding() const;

  /**
   * Moves the anchor by whole refreshes, a Fixed clock paying its tally when a frame is owed
   */
  void stepOnTheGrid();

  /**
   * Moves the anchor a bounded step toward the target phase of the refresh grid
   */
  void correctPhase();

  /**
   * The target-phase instant of the refresh the frame at anchorNs would reach anyway
   */
  [[nodiscard]] int64_t targetInstantFor(int64_t anchorNs) const;

  /**
   * Recomputes where a frame is asked for from what the frame, the pass and the wake take
   */
  void updatePhaseTarget();

  PacingContext m_context;

  /** What Auto chose, or the configured mode as given. Never Auto */
  SyncMode m_resolvedMode = SyncMode::Fixed;

  double m_effectiveFps = 60.0;
  double m_audioRatio = 1.0;
  int m_refreshesPerFrame = 0;
  int m_heldRefreshes = 0;

  /** When the next frame is due, or 0 before the first */
  /** The refresh the rates in force were worked out from */
  int64_t m_ratesFromPeriodNs = 0;

  int64_t m_anchorNs = 0;
  int64_t m_periodNs = 0;
  int64_t m_displayPeriodNs = 0;

  /** Whether the last tick stepped on the grid, so coming under the hold or leaving it is noticed */
  bool m_wasHolding = false;

  /** Whether the anchor should be put on the target the next time it is moved */
  bool m_snapPending = false;

  /** How much of the content's time a held Fixed clock has not yet run */
  int64_t m_slipDebtNs = 0;

  /** How long a frame takes from its tick to its picture, smoothed */
  int64_t m_frameTimeNs = 0;

  // TODO
  /**
   * The typical slow frame and the typical quick one, the frames over and under the smoothed time
   * each smoothed on their own, and how far the slow ones stray from theirs
   */
  int64_t m_slowestFrameNs = 0;
  int64_t m_quickestFrameNs = 0;
  int64_t m_slowSpreadNs = 0;

  /** How long a pass takes to put a picture on the target, smoothed */
  int64_t m_passTimeNs = 0;

  /** How late the loop has recently woken at worst, decaying */
  int64_t m_wakePeakNs = 0;

  double m_phaseTarget = PHASE_TARGET;

  /** A target set by hand, which the measurements do not move */
  double m_pinnedPhaseTarget = -1.0;

  PhaseEstimator m_phase;

  monitoring::Marker m_slipMarker = monitoring::Monitor::instance().marker(
      "slip", "A held clock ran a frame twice in one refresh, or none, to keep the content's rate");
  // TODO
  monitoring::Marker m_timeDroppedMarker = monitoring::Monitor::instance().marker(
      "time_dropped", "A stall dropped the time it took rather than running the frames it covered");
  monitoring::Series m_slipDebtSeries = monitoring::Monitor::instance().series(
      "slip_debt", "What a held clock still owes the content's rate, in milliseconds");
  monitoring::Series m_phaseLockedSeries =
      monitoring::Monitor::instance().series("phase_locked", "Whether the presents have agreed on a grid, 1 or 0");
  monitoring::Series m_refreshPeriodSeries = monitoring::Monitor::instance().series(
      "refresh_period", "The refresh period the presents measure, in milliseconds");

  /** 0..1, or negative when the sink can't be read */
  double m_audioBufferLevel = -1.0;
};

} // namespace firelight::emulation

// TODO: NEEDS REVIEW
#pragma once

#include <cstdint>

namespace firelight::emulation {

/**
 * Tracks the display's refresh grid from the instants frames are handed to it.
 *
 * A phase-locked loop on the present train: each present is compared with where the grid predicted
 * it, the grid moves a little toward it and the period a little less. Holds no thread and reads no
 * clock; the caller passes every instant in
 */
class PhaseEstimator {
public:
  /** How much of a present's error the grid takes on */
  static constexpr double PHASE_GAIN = 0.05;

  /** How much of a present's error, per refresh it spanned, the period takes on */
  static constexpr double PERIOD_GAIN = 0.005;

  /**
   * How far the measured period may stray from the one the display reports, as a fraction. Wide
   * enough for a panel that reports a rate it does not run at, which is what measuring is for
   */
  static constexpr double PERIOD_TOLERANCE = 0.25;

  /**
   * How near its prediction a present has to land to count toward the lock, as a fraction of a
   * period. Tight enough that a train arriving in pairs a few milliseconds apart never counts as a
   * grid
   */
  static constexpr double LOCK_WINDOW = 0.1;

  // TODO
  /**
   * The narrowest that window may be, in ns. A tenth of a refresh is 1.7 ms at 60 Hz but 0.4 ms at
   * 240 Hz, which is the same size as the jitter a present train carries, so on a fast panel the
   * window would close on the noise and the grid would never be trusted
   */
  static constexpr int64_t LOCK_WINDOW_FLOOR_NS = 500000;

  /** How many consecutive presents inside the window make the grid trustworthy */
  static constexpr int LOCK_PRESENTS = 30;

  /** What a present outside the window costs the count that earns the lock */
  static constexpr int UNLOCK_MISSES = 5;

  // TODO
  /**
   * How far the count has to fall before a grid that was trusted is not. Below the level that earns
   * it, so one present in the wrong place does not take the grid away and give it straight back
   */
  static constexpr int RELOCK_PRESENTS = 20;

  /** A gap longer than this means the window stopped drawing, and the grid starts over */
  static constexpr int64_t RESYNC_NS = 250000000;

  /**
   * Sets the refresh period the display reports and starts over
   */
  void configure(int64_t nominalPeriodNs);

  /**
   * Feeds one present. Presents inside one refresh of the previous one are pipelined submits and are
   * left out; a gap over RESYNC_NS restarts the grid at this present
   */
  void notePresent(int64_t nowNs);

  /**
   * @return Whether enough presents have landed where the grid predicted to trust it
   */
  [[nodiscard]] bool isLocked() const;

  /**
   * @return The measured refresh period, or the nominal one before any present
   */
  [[nodiscard]] int64_t getPeriodNs() const;

  /**
   * @return The estimated refresh instant nearest nearNs, or nearNs itself before any present
   */
  [[nodiscard]] int64_t getGridNs(int64_t nearNs) const;

  /**
   * Forgets the grid and the lock, keeping the nominal period
   */
  void reset();

private:
  int64_t m_nominalPeriodNs = 0;
  double m_periodNs = 0.0;
  double m_gridNs = 0.0;
  int64_t m_lastPresentNs = 0;
  int m_inWindow = 0;
  bool m_isLocked = false;
};

} // namespace firelight::emulation

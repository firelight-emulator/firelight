// TODO: NEEDS REVIEW
#pragma once

#include <cstdint>

namespace firelight::emulation {

/**
 * Waits until a deadline as accurately as the host allows, and learns how much of it to spin.
 *
 * A sleep does not return when it was asked to. How late it is varies by operating system, by
 * whether a fine-grained timer could be had, and by what else is running, so the part of the wait
 * that is spun rather than slept is measured rather than assumed
 */
class PrecisionWaiter {
public:
  // TODO
  // The least of a wait ever spun, and the most. The floor is what a sleep is still allowed to be
  // late by without widening anything; the ceiling keeps a host with a bad timer from burning a
  // whole core to hide it
  static constexpr int64_t MIN_SPIN_MARGIN_NS = 500000;
  static constexpr int64_t MAX_SPIN_MARGIN_NS = 8000000;

  // TODO
  // Added to a measured overshoot before it becomes the margin, so that the margin covers the sleeps
  // it was widened for rather than landing exactly on them
  static constexpr int64_t SPIN_MARGIN_HEADROOM_NS = 250000;

  // TODO
  // How slowly the margin gives back what a bad sleep bought it. The sleeps that matter are the rare
  // worst ones, and a margin that sags back between them does not cover them when they come
  static constexpr int64_t DECAY_DIVISOR = 4096;

  PrecisionWaiter() = default;
  ~PrecisionWaiter();

  PrecisionWaiter(const PrecisionWaiter &) = delete;
  PrecisionWaiter &operator=(const PrecisionWaiter &) = delete;

  /**
   * How much of the wait is currently being spun rather than slept
   */
  [[nodiscard]] int64_t getSpinMarginNs() const { return m_spinMarginNs; }

  /**
   * Sleeps for close to this long, returning false where the host offers nothing accurate enough and
   * the caller should fall back to its own wait
   */
  bool sleepFor(int64_t durationNs);

  /**
   * Widens or narrows the spun part to cover how late a sleep just returned.
   *
   * Widened at once, because the wait that overshot is already late and so is every one after it
   * until it is covered; narrowed slowly, so one accurate sleep among late ones does not give back
   * the margin the late ones bought
   */
  void noteOvershoot(int64_t overshootNs);

private:
  /**
   * The host's finest timer, opened on first use and held for the life of this object
   */
  void *m_timer = nullptr;
  bool m_timerAttempted = false;

  int64_t m_spinMarginNs = MIN_SPIN_MARGIN_NS;
};

} // namespace firelight::emulation

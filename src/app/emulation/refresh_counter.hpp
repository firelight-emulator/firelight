// TODO: NEEDS REVIEW
#pragma once

#include <cstdint>

namespace firelight::emulation {

// TODO
/**
 * Reads the gap between two presents as a number of display refreshes
 */
class RefreshCounter {
public:
  // TODO
  /**
   * The smallest the ceiling may be, so that a frame held for only a refresh or two still has room
   * to make up a hitch
   */
  static constexpr int MIN_CEILING = 4;

  // TODO
  /**
   * The most refreshes one present may be read as covering, for a frame held for refreshesPerFrame
   * of them. One more than a whole frame, so a present arriving a refresh late is still counted in
   * full, while no single present can ever be worth more than one extra frame
   */
  [[nodiscard]] static int ceilingFor(int refreshesPerFrame);

  // TODO
  /**
   * Counts the whole refreshes a gap covers, carrying the part left over to the next call so that
   * jitter around a boundary neither gains nor loses refreshes over time.
   *
   * @param ceiling The most this call may return, and also the most that may be carried
   * @return At least one and at most ceiling; one when the period isn't known
   */
  [[nodiscard]] int observe(int64_t gapNs, int64_t periodNs, int ceiling);

  // TODO
  /**
   * Forgets the carried part
   */
  void reset();

private:
  // TODO
  /** The part of a refresh left over after counting whole ones */
  double m_remainder = 0.0;
};

} // namespace firelight::emulation

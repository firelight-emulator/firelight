#include "refresh_counter.hpp"

#include <algorithm>

namespace firelight::emulation {

int RefreshCounter::ceilingFor(const int refreshesPerFrame) { return std::max(MIN_CEILING, refreshesPerFrame + 1); }

int RefreshCounter::observe(const int64_t gapNs, const int64_t periodNs, const int ceiling) {
  if (periodNs <= 0) {
    return 1;
  }

  const auto bound = std::max(1, ceiling);
  const auto limit = static_cast<double>(bound);

  // TODO
  // What is carried is bounded, not just what is returned. A count that stops at the ceiling while
  // the part left over keeps growing reads every later present short by the same amount, so a frame
  // held for more refreshes than the ceiling never gets all of them; and time the window spent not
  // drawing at all would come back afterwards as a run of frames nobody is waiting for
  m_remainder = std::clamp(m_remainder + static_cast<double>(gapNs) / static_cast<double>(periodNs), -limit, limit);

  const auto refreshes = std::clamp(static_cast<int>(m_remainder), 1, bound);
  m_remainder -= refreshes;

  return refreshes;
}

void RefreshCounter::reset() { m_remainder = 0.0; }

} // namespace firelight::emulation

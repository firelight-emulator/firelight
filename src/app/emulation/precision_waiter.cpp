// TODO: NEEDS REVIEW
#include "precision_waiter.hpp"

#include <algorithm>

#ifdef _WIN32
// clang-format off
#include <windows.h>
// clang-format on
#endif

namespace firelight::emulation {

PrecisionWaiter::~PrecisionWaiter() {
#ifdef _WIN32
  if (m_timer != nullptr) {
    CloseHandle(static_cast<HANDLE>(m_timer));
  }
#endif
}

bool PrecisionWaiter::sleepFor(const int64_t durationNs) {
  if (durationNs <= 0) {
    return false;
  }

#ifdef _WIN32
  if (!m_timerAttempted) {
    m_timerAttempted = true;
    // TODO
    // Windows hands out ordinary sleeps on a tick that can be 15.6ms, which no margin short of a
    // whole frame covers. The high-resolution timer has been available since 1803
    m_timer = CreateWaitableTimerExW(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
  }

  if (m_timer == nullptr) {
    return false;
  }

  LARGE_INTEGER dueTime;
  // Negative is relative, in hundreds of nanoseconds
  dueTime.QuadPart = -(durationNs / 100);

  if (dueTime.QuadPart >= 0 ||
      !SetWaitableTimerEx(static_cast<HANDLE>(m_timer), &dueTime, 0, nullptr, nullptr, nullptr, 0)) {
    return false;
  }

  return WaitForSingleObject(static_cast<HANDLE>(m_timer), INFINITE) == WAIT_OBJECT_0;
#else
  return false;
#endif
}

void PrecisionWaiter::noteOvershoot(const int64_t overshootNs) {
  // TODO
  // Held rather than widened when it already covers exactly, so a host that is late by the same
  // amount every time settles instead of decaying and re-widening forever
  if (overshootNs + SPIN_MARGIN_HEADROOM_NS >= m_spinMarginNs) {
    m_spinMarginNs = std::min(overshootNs + SPIN_MARGIN_HEADROOM_NS, MAX_SPIN_MARGIN_NS);
    return;
  }

  m_spinMarginNs =
      std::max(m_spinMarginNs - (m_spinMarginNs - MIN_SPIN_MARGIN_NS) / DECAY_DIVISOR - 1, MIN_SPIN_MARGIN_NS);
}

} // namespace firelight::emulation

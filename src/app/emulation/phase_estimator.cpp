// TODO: NEEDS REVIEW
#include "phase_estimator.hpp"

#include <algorithm>
#include <cmath>

namespace firelight::emulation {

void PhaseEstimator::configure(const int64_t nominalPeriodNs) {
  m_nominalPeriodNs = nominalPeriodNs;
  reset();
}

void PhaseEstimator::reset() {
  m_periodNs = static_cast<double>(m_nominalPeriodNs);
  m_gridNs = 0.0;
  m_lastPresentNs = 0;
  m_inWindow = 0;
  m_isLocked = false;
}

void PhaseEstimator::notePresent(const int64_t nowNs) {
  if (m_nominalPeriodNs <= 0) {
    return;
  }

  if (m_lastPresentNs == 0 || nowNs - m_lastPresentNs > RESYNC_NS) {
    m_gridNs = static_cast<double>(nowNs);
    m_lastPresentNs = nowNs;
    m_inWindow = 0;
    m_isLocked = false;
    return;
  }

  const auto refreshes = std::lround((static_cast<double>(nowNs) - m_gridNs) / m_periodNs);

  // TODO
  // A second present inside the same refresh is the swapchain pipelining submits, not a refresh
  if (refreshes < 1) {
    m_lastPresentNs = nowNs;
    return;
  }

  const auto predictedNs = m_gridNs + static_cast<double>(refreshes) * m_periodNs;
  const auto errorNs = static_cast<double>(nowNs) - predictedNs;

  m_gridNs = predictedNs + PHASE_GAIN * errorNs;
  m_periodNs += PERIOD_GAIN * errorNs / static_cast<double>(refreshes);
  m_periodNs = std::clamp(m_periodNs, static_cast<double>(m_nominalPeriodNs) * (1.0 - PERIOD_TOLERANCE),
                          static_cast<double>(m_nominalPeriodNs) * (1.0 + PERIOD_TOLERANCE));

  const auto windowNs = std::max(LOCK_WINDOW * m_periodNs, static_cast<double>(LOCK_WINDOW_FLOOR_NS));

  // TODO
  // A present where the grid predicted earns one toward the lock; one anywhere else costs several, so
  // a train that only lands there now and then never earns it
  if (std::abs(errorNs) <= windowNs) {
    m_inWindow = std::min(m_inWindow + 1, LOCK_PRESENTS);
  } else {
    m_inWindow = std::max(m_inWindow - UNLOCK_MISSES, 0);
  }

  if (m_inWindow >= LOCK_PRESENTS) {
    m_isLocked = true;
  } else if (m_inWindow < RELOCK_PRESENTS) {
    m_isLocked = false;
  }

  m_lastPresentNs = nowNs;
}

bool PhaseEstimator::isLocked() const { return m_isLocked; }

int64_t PhaseEstimator::getPeriodNs() const { return static_cast<int64_t>(std::llround(m_periodNs)); }

int64_t PhaseEstimator::getGridNs(const int64_t nearNs) const {
  if (m_lastPresentNs == 0 || m_periodNs <= 0.0) {
    return nearNs;
  }

  const auto refreshes = std::round((static_cast<double>(nearNs) - m_gridNs) / m_periodNs);
  return static_cast<int64_t>(std::llround(m_gridNs + refreshes * m_periodNs));
}

} // namespace firelight::emulation

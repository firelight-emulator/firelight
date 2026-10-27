#include "firelight/util/timer.hpp"

namespace firelight::util {

Timer::Timer() = default;

void Timer::start() {
  if (!m_running) {
    m_startTimeMillis = m_nowSupplier();
    m_running = true;
  }
}

std::chrono::milliseconds Timer::pause() {
  if (m_running) {
    m_elapsedTimeMillis += m_nowSupplier() - m_startTimeMillis;
    m_running = false;
  }

  return std::chrono::duration_cast<std::chrono::milliseconds>(m_elapsedTimeMillis);
}

std::chrono::milliseconds Timer::restart() {
  const auto now = m_nowSupplier();

  std::chrono::steady_clock::duration elapsed = m_elapsedTimeMillis;
  if (m_running) {
    elapsed += now - m_startTimeMillis;
  }

  m_startTimeMillis = now;
  m_elapsedTimeMillis = std::chrono::milliseconds(0);

  m_running = true;
  return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
}

std::chrono::milliseconds Timer::elapsed() const {
  auto total = m_elapsedTimeMillis;
  if (m_running) {
    total += m_nowSupplier() - m_startTimeMillis;
  }

  return std::chrono::duration_cast<std::chrono::milliseconds>(total);
}

std::chrono::milliseconds Timer::timeSinceEpoch() const {
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
}

} // namespace firelight::util
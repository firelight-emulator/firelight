#pragma once

#include <chrono>
#include <functional>

namespace firelight::util {
class Timer {
public:
  Timer();

  // This is just for testing. Default constructor uses steady_clock::now()
  explicit Timer(std::function<std::chrono::steady_clock::time_point()> nowSupplier) : m_nowSupplier(std::move(nowSupplier)) {}

  //Starts the timer if it is not already running
  void start();

  // Pauses the timer and returns the total elapsed time in milliseconds.
  // If the timer is already paused, returns the total elapsed time without changing it
  std::chrono::milliseconds pause();

  // Restarts the timer and returns the total elapsed time in milliseconds
  std::chrono::milliseconds restart();

  // Returns the total elapsed time in milliseconds
  [[nodiscard]] std::chrono::milliseconds elapsed() const;

  // Returns unix epoch millis (since Jan 1, 1970). Mostly for convenience
  std::chrono::milliseconds timeSinceEpoch() const;

private:
  bool m_running = false;
  std::chrono::steady_clock::time_point m_startTimeMillis;
  std::chrono::steady_clock::duration m_elapsedTimeMillis{};

  std::function<std::chrono::steady_clock::time_point()> m_nowSupplier = [] {
    return std::chrono::steady_clock::now();
  };
};
} // namespace firelight::util
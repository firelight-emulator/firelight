// TODO: NEEDS REVIEW
#pragma once

#include <firelight/monitoring/monitor.hpp>

#include <atomic>
#include <string>
#include <thread>

namespace firelight::diagnostics {

/**
 * Marks every vertical blank of one display in the monitor, from a thread of its own
 */
class VblankProbe {
public:
  VblankProbe() = default;

  ~VblankProbe();

  VblankProbe(const VblankProbe &) = delete;

  VblankProbe &operator=(const VblankProbe &) = delete;

  /**
   * Follows the display with this name, restarting when the name changes. Does nothing off Windows
   */
  void follow(const std::string &displayName);

  void stop();

private:
  void run(const std::string &displayName) const;

  std::thread m_thread;
  std::atomic<bool> m_running{false};
  std::string m_displayName;

  monitoring::Marker m_vblankMarker =
      monitoring::Monitor::instance().marker("vblank", "The display began a vertical blank");
};

} // namespace firelight::diagnostics

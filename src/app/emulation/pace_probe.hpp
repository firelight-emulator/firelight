// TODO: NEEDS REVIEW
#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <spdlog/spdlog.h>

namespace firelight::emulation {

// TODO
/**
 * TEMP: counts everything between a tick and a picture, once a second, under FL_PACE_PROBE=1.
 *
 * Presents and passes are not the same event and neither is a sync, which is the only one that
 * delivers frames. Counting all of them separately is what tells a starved pass apart from a
 * missing one
 */
class PaceProbe {
public:
  // TODO
  /**
   * @return The one instance, since the threads that count reach this without anywhere to inject it
   */
  static PaceProbe &instance() {
    static PaceProbe probe;
    return probe;
  }

  // TODO
  /**
   * @return Whether the probe was asked for, read once
   */
  static bool isEnabled() {
    static const bool enabled = std::getenv("FL_PACE_PROBE") != nullptr;
    return enabled;
  }

  // TODO
  /** Every return from the loop's wait, whether or not anything was due */
  std::atomic<int> wakes{0};

  // TODO
  /** A loop iteration that asked for at least one frame */
  std::atomic<int> ticks{0};

  // TODO
  /** RunFrame commands that reached the renderer */
  std::atomic<int> framesRequested{0};

  // TODO
  /** frameSwapped emissions */
  std::atomic<int> presents{0};

  // TODO
  /** synchronize() calls, which are the only ones that drain commands */
  std::atomic<int> syncs{0};

  // TODO
  /** render() calls, including the ones with nothing to draw */
  std::atomic<int> renders{0};

  // TODO
  /** render() calls that found nothing owed */
  std::atomic<int> idleRenders{0};

  // TODO
  /** Frames the core actually advanced through */
  std::atomic<int> framesRun{0};

  // TODO
  /**
   * Records how many frames one pass carried, so a rate that looks right can still be seen to
   * arrive two at a time
   */
  void notePass(const int framesThisPass) {
    const auto bucket = framesThisPass < 0                            ? 0
                        : framesThisPass >= static_cast<int>(HISTOGRAM_SIZE) ? HISTOGRAM_SIZE - 1
                                                                        : framesThisPass;
    m_passHistogram[bucket].fetch_add(1);
  }

  // TODO
  /**
   * Prints a second's worth of counts and starts the next second, doing nothing until one is up
   */
  void reportIfDue(const int64_t nowNs) {
    auto lastNs = m_lastReportNs.load();

    if (lastNs == 0) {
      m_lastReportNs.store(nowNs);
      return;
    }

    if (nowNs - lastNs < REPORT_INTERVAL_NS) {
      return;
    }

    if (!m_lastReportNs.compare_exchange_strong(lastNs, nowNs)) {
      return;
    }

    std::string passes;
    for (auto bucket = 0u; bucket < HISTOGRAM_SIZE; ++bucket) {
      if (const auto count = m_passHistogram[bucket].exchange(0); count > 0) {
        passes += fmt::format(" {}x{}", bucket, count);
      }
    }

    spdlog::info("pace: wakes={} ticks={} req={} presents={} syncs={} renders={} idle={} frames={} | pass{}",
                 wakes.exchange(0), ticks.exchange(0), framesRequested.exchange(0), presents.exchange(0), syncs.exchange(0),
                 renders.exchange(0), idleRenders.exchange(0), framesRun.exchange(0),
                 passes.empty() ? " none" : passes);
  }

private:
  // TODO
  /** Frames in one pass past this are all counted together */
  static constexpr unsigned HISTOGRAM_SIZE = 6;

  // TODO
  /** How much of a second a report covers */
  static constexpr int64_t REPORT_INTERVAL_NS = 1000000000;

  std::array<std::atomic<int>, HISTOGRAM_SIZE> m_passHistogram{};
  std::atomic<int64_t> m_lastReportNs{0};
};

} // namespace firelight::emulation

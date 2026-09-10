// TODO
// How late input can be polled before frames start missing their refresh. The frame is scheduled to
// COMPLETE at the target phase and input is polled at its start, so a later target means fresher
// input. What decides how late the target can go is how much the core's work time varies
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <initializer_list>
#include <numeric>
#include <random>
#include <vector>

namespace {

// TODO
/** Long enough for rare spikes to appear a useful number of times */
constexpr double RUN_SECONDS = 300.0;

// TODO
/** Samples skipped before latency is recorded, while the phase settles */
constexpr int WARMUP_SAMPLES = 300;

// TODO
/** How much of the phase error is corrected per frame */
constexpr double PHASE_CORRECTION_DIVISOR = 64.0;

// TODO
/** Spread on the core's work time, as a fraction of its typical value */
constexpr double WORK_SPREAD = 0.25;

/**
 * Input-to-scanout latency for one target, and what it cost
 */
struct Outcome {
  double meanMs = 0.0;
  double p99Ms = 0.0;
  double worstMs = 0.0;
  /** Refreshes that re-showed the previous frame */
  int repeats = 0;
  /** Frames produced that no refresh ever showed */
  int lost = 0;
};

/**
 * What the core costs per frame. Spikes stand in for a shader compile, a savestate, or a heavy
 * scanline — the frames that decide whether an aggressive target is safe
 */
struct Load {
  const char *name;
  double workMs;
  double spikeMs;
  double spikeRate;
};

/**
 * Runs the emulation clock phase-locked to the given target, publishing into a latest-frame slot the
 * display samples at each refresh
 */
Outcome run(const double displayHz, const double target, const Load &load, const unsigned seed) {
  const auto period = 1.0 / displayHz;
  std::mt19937 rng(seed);
  std::uniform_real_distribution spike(0.0, 1.0);
  std::normal_distribution jitter(0.0, load.workMs * WORK_SPREAD);

  auto anchor = 0.0;
  auto nextRefresh = 0.0;
  auto lastRefresh = 0.0;
  auto polledAt = 0.0;
  auto slotId = 0;
  auto lastShownId = -1;
  auto pending = 0;
  auto samples = 0;
  std::vector<double> latencies;
  Outcome outcome;

  for (auto now = 0.0; now < RUN_SECONDS;) {
    if (anchor <= nextRefresh) {
      auto workMs = (spike(rng) < load.spikeRate ? load.spikeMs : load.workMs) + jitter(rng);
      workMs = std::max(0.05, workMs);
      const auto work = workMs / 1000.0;

      // TODO
      // The anchor is when the frame must be done, so it starts a work's length before that. Input is
      // polled at the start, which is what makes a later target fresher input
      polledAt = anchor - work;
      const auto readyAt = anchor;

      if (pending > 0) {
        outcome.lost++;
      }

      slotId++;
      pending = 1;
      anchor += period;

      if (readyAt - anchor > period) {
        anchor = readyAt;
      }

      auto phase = std::fmod(anchor - lastRefresh, period);

      if (phase < 0.0) {
        phase += period;
      }

      anchor += (period * target - phase) / PHASE_CORRECTION_DIVISOR;
      now = readyAt;
      continue;
    }

    if (slotId != lastShownId) {
      lastShownId = slotId;
      pending = 0;

      if (++samples > WARMUP_SAMPLES) {
        latencies.push_back((nextRefresh - polledAt) * 1000.0);
      }
    } else {
      outcome.repeats++;
    }

    lastRefresh = nextRefresh;
    nextRefresh += period;
    now = nextRefresh;
  }

  std::ranges::sort(latencies);
  outcome.meanMs = std::reduce(latencies.begin(), latencies.end()) / static_cast<double>(latencies.size());
  outcome.p99Ms = latencies[static_cast<size_t>(static_cast<double>(latencies.size()) * 0.99)];
  outcome.worstMs = latencies.back();

  return outcome;
}

} // namespace

int main() {
  constexpr auto DISPLAY_HZ = 59.959;

  std::printf("%.3f Hz, %.0f seconds per run. Input is polled at frame start; latency is poll to scanout.\n",
              DISPLAY_HZ, RUN_SECONDS);
  std::printf("Queue depth adds on top. A frame that overruns lands on the next refresh rather than\n");
  std::printf("being dropped, so the cost of an aggressive target shows up in the tail, not in 'lost'.\n\n");

  for (const auto &load : {Load{"steady 1ms core", 1.0, 1.0, 0.0}, Load{"1ms core, 4ms spikes at 1%", 1.0, 4.0, 0.01},
                           Load{"3ms core, 8ms spikes at 2%", 3.0, 8.0, 0.02}}) {
    std::printf("%s\n  %-8s %9s %9s %9s %9s %8s\n", load.name, "target", "mean ms", "p99 ms", "worst", "repeats",
                "lost");

    for (const auto target : {0.50, 0.75, 0.90, 0.98}) {
      const auto outcome = run(DISPLAY_HZ, target, load, 7);
      std::printf("  %-7.0f%% %9.2f %9.2f %9.2f %9d %8d\n", target * 100, outcome.meanMs, outcome.p99Ms,
                  outcome.worstMs, outcome.repeats, outcome.lost);
    }

    std::printf("\n");
  }

  return 0;
}

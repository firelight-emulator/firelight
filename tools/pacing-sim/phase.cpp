// TODO
// Two questions about the phase term: what a wrong reported refresh rate costs a clock-paced
// emulator, and what the phase target is worth in latency once the phase is controlled
#include <cmath>
#include <cstdio>
#include <initializer_list>
#include <random>

namespace {

// TODO
/** Long enough for a slow drift to cross a refresh boundary several times */
constexpr double DRIFT_SECONDS = 300.0;

// TODO
/** Shorter, because latency settles in seconds rather than minutes */
constexpr double LATENCY_SECONDS = 120.0;

// TODO
/** Scheduling noise on the emulation clock, as a fraction of one frame */
constexpr double CLOCK_NOISE = 0.02;

// TODO
/** How much of the phase error is corrected per frame. Slow enough not to fight ordinary jitter */
constexpr double PHASE_CORRECTION_DIVISOR = 64.0;

// TODO
/** How quickly the observed vblank period is averaged */
constexpr double VBLANK_AVERAGE_DIVISOR = 256.0;

// TODO
/** How long the core is assumed to take, for the latency figures */
constexpr double CORE_WORK_SECONDS = 0.001;

struct DriftOutcome {
  int shown = 0;
  int repeats = 0;
  int dropped = 0;
};

/**
 * Runs the emulation clock at the display's reported rate while the display runs at its actual one.
 * With phase-locking the anchor is nudged toward a fixed point in the refresh interval, using only
 * the observed vblank grid — a rate is never taken from it
 */
DriftOutcome runDrift(const double actualHz, const double reportedHz, const bool phaseLock, const unsigned seed) {
  const auto refreshPeriod = 1.0 / actualHz;
  const auto emuPeriod = 1.0 / reportedHz;
  std::mt19937 rng(seed);
  std::uniform_real_distribution noise(-CLOCK_NOISE, CLOCK_NOISE);

  auto anchor = 0.0;
  auto nextRefresh = 0.0;
  auto vblankEstimate = refreshPeriod;
  auto lastRefreshSeen = 0.0;
  auto slotId = 0;
  auto lastShownId = -1;
  auto pending = 0;
  DriftOutcome outcome;

  for (auto now = 0.0; now < DRIFT_SECONDS;) {
    if (anchor <= nextRefresh) {
      const auto ranAt = anchor + noise(rng) * emuPeriod;

      if (pending > 0) {
        outcome.dropped++;
      }

      slotId++;
      pending = 1;
      anchor += emuPeriod;

      if (ranAt - anchor > emuPeriod) {
        anchor = ranAt;
      }

      if (phaseLock && lastRefreshSeen > 0.0) {
        auto phase = std::fmod(anchor - lastRefreshSeen, vblankEstimate);

        if (phase < 0.0) {
          phase += vblankEstimate;
        }

        anchor += (vblankEstimate * 0.75 - phase) / PHASE_CORRECTION_DIVISOR;
      }

      now = ranAt;
      continue;
    }

    if (slotId != lastShownId) {
      outcome.shown++;
      lastShownId = slotId;
      pending = 0;
    } else {
      outcome.repeats++;
    }

    lastRefreshSeen = nextRefresh;
    vblankEstimate += (refreshPeriod - vblankEstimate) / VBLANK_AVERAGE_DIVISOR;
    nextRefresh += refreshPeriod;
    now = nextRefresh;
  }

  return outcome;
}

/**
 * Measures input-poll-to-scanout for a phase target, once the phase is under control. Queue depth
 * adds on top of whatever this reports
 */
void reportLatency(const double displayHz, const double target, const unsigned seed) {
  const auto period = 1.0 / displayHz;
  std::mt19937 rng(seed);
  std::uniform_real_distribution noise(-CLOCK_NOISE, CLOCK_NOISE);

  auto anchor = 0.0;
  auto nextRefresh = 0.0;
  auto lastRefresh = 0.0;
  auto slotId = 0;
  auto lastShownId = -1;
  auto polledAt = 0.0;
  auto sum = 0.0;
  auto lowest = 1e9;
  auto highest = -1e9;
  auto samples = 0;
  auto dropped = 0;
  auto pending = 0;

  // TODO
  // The first frames are skipped, because the phase has not settled and would drag the mean
  constexpr auto WARMUP_SAMPLES = 200;

  for (auto now = 0.0; now < LATENCY_SECONDS;) {
    if (anchor <= nextRefresh) {
      const auto ranAt = anchor + noise(rng) * period;

      if (pending > 0) {
        dropped++;
      }

      slotId++;
      pending = 1;
      polledAt = ranAt - CORE_WORK_SECONDS;
      anchor += period;

      if (ranAt - anchor > period) {
        anchor = ranAt;
      }

      auto phase = std::fmod(anchor - lastRefresh, period);

      if (phase < 0.0) {
        phase += period;
      }

      anchor += (period * target - phase) / PHASE_CORRECTION_DIVISOR;
      now = ranAt;
      continue;
    }

    if (slotId != lastShownId) {
      lastShownId = slotId;
      pending = 0;
      samples++;

      if (samples > WARMUP_SAMPLES) {
        const auto latencyMs = (nextRefresh - polledAt) * 1000.0;
        sum += latencyMs;
        lowest = std::fmin(lowest, latencyMs);
        highest = std::fmax(highest, latencyMs);
      }
    }

    lastRefresh = nextRefresh;
    nextRefresh += period;
    now = nextRefresh;
  }

  std::printf("  %-12.0f%% %10.2f %10.2f %10.2f %8d\n", target * 100, sum / (samples - WARMUP_SAMPLES), lowest, highest,
              dropped);
}

struct DriftCase {
  const char *name;
  double reportedHz;
  double actualHz;
};

} // namespace

int main() {
  std::printf("The emulation clock is set to the display's reported rate; the display runs at its actual one.\n");
  std::printf("%.0f seconds per run. 'lost' is frames produced that no refresh ever showed.\n\n", DRIFT_SECONDS);
  std::printf("  %-32s %-14s %8s %8s %8s\n", "case", "policy", "shown", "repeat", "lost");

  for (const auto &driftCase : {DriftCase{"exact match (unrealistic)", 59.959, 59.959},
                                DriftCase{"display 50 ppm fast", 59.959, 59.959 * 1.00005},
                                DriftCase{"display 100 ppm slow", 59.959, 59.959 * 0.9999},
                                DriftCase{"reported 60.000, real 59.94", 60.000, 59.94},
                                DriftCase{"reported 60.000, real 59.959", 60.000, 59.959}}) {
    for (const auto phaseLock : {false, true}) {
      const auto outcome = runDrift(driftCase.actualHz, driftCase.reportedHz, phaseLock, 3);
      std::printf("  %-32s %-14s %8d %8d %8d\n", phaseLock ? "" : driftCase.name,
                  phaseLock ? "phase-lock" : "clock only", outcome.shown, outcome.repeats, outcome.dropped);
    }

    std::printf("\n");
  }

  std::printf("Input poll to the refresh that scans the frame out, %.0f seconds per target.\n", LATENCY_SECONDS);
  std::printf("Queue depth adds on top of these.\n\n");
  std::printf("  %-12s %10s %10s %10s %8s\n", "target", "mean ms", "min ms", "max ms", "lost");

  for (const auto target : {0.10, 0.25, 0.50, 0.75, 0.90, 0.98}) {
    reportLatency(59.959, target, 11);
  }

  return 0;
}

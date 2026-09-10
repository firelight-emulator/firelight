// TODO
// Compares the two candidate pacing policies against the same modelled display. Neither exists in
// the tree yet, so both are modelled rather than linked
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <initializer_list>
#include <random>
#include <vector>

namespace {

// TODO
/** How long each run covers, long enough for a 7-second beat to show several times */
constexpr double RUN_SECONDS = 60.0;

// TODO
/** Scheduling noise on the emulation clock, as a fraction of one frame */
constexpr double CLOCK_NOISE = 0.05;

/**
 * What a run looked like from the player's side
 */
struct Outcome {
  int framesRun = 0;
  /** Refreshes that showed a frame the previous refresh had not */
  int shown = 0;
  /** Refreshes that re-showed the previous frame */
  int repeats = 0;
  /** Frames produced that no refresh ever showed */
  int dropped = 0;
};

/**
 * Emulation on its own thread at a fixed rate, publishing into a latest-frame slot the display
 * samples at each refresh. Debt is dropped rather than repaid, as every reference implementation does
 */
Outcome runClock(const double displayHz, const double emuFps, const unsigned seed) {
  const auto refreshPeriod = 1.0 / displayHz;
  const auto emuPeriod = 1.0 / emuFps;
  std::mt19937 rng(seed);
  std::uniform_real_distribution noise(-CLOCK_NOISE, CLOCK_NOISE);

  auto anchor = 0.0;
  auto nextRefresh = 0.0;
  auto slotId = 0;
  auto lastShownId = -1;
  auto pending = 0;
  Outcome outcome;

  for (auto now = 0.0; now < RUN_SECONDS;) {
    if (anchor <= nextRefresh) {
      const auto ranAt = anchor + noise(rng) * emuPeriod;

      if (pending > 0) {
        // TODO
        // The slot was overwritten before any refresh sampled it
        outcome.dropped++;
      }

      slotId++;
      pending = 1;
      outcome.framesRun++;

      const auto due = anchor + emuPeriod;
      anchor = ranAt - due < emuPeriod ? due : ranAt;
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

    nextRefresh += refreshPeriod;
    now = nextRefresh;
  }

  return outcome;
}

/**
 * Every refresh is one render pass; a frame runs every k passes. No clock and no timestamps are
 * involved at all, which is what RetroArch does with vsync on
 */
Outcome runPassCounting(const double displayHz, const int refreshesPerFrame) {
  const auto refreshes = static_cast<int>(displayHz * RUN_SECONDS);
  auto passes = 0;
  Outcome outcome;

  for (auto refresh = 0; refresh < refreshes; ++refresh) {
    if (++passes < refreshesPerFrame) {
      outcome.repeats++;
      continue;
    }

    passes = 0;
    outcome.framesRun++;
    outcome.shown++;
  }

  return outcome;
}

/**
 * @return How many whole refreshes land nearest one content frame
 */
int refreshesPerFrame(const double displayHz, const double contentFps) {
  return std::max(1, static_cast<int>(std::lround(displayHz / contentFps)));
}

void report(const char *policy, const Outcome &outcome) {
  std::printf("  %-24s ran=%6.2ffps shown=%5d repeat=%5d dropped=%4d\n", policy, outcome.framesRun / RUN_SECONDS,
              outcome.shown, outcome.repeats, outcome.dropped);
}

struct Scenario {
  const char *name;
  double displayHz;
  double contentFps;
};

} // namespace

int main() {
  const std::vector<Scenario> scenarios = {
      {"SNES 60.099 @ 59.959", 59.959, 60.0988},   {"SNES 60.099 @ 60.000", 60.000, 60.0988},
      {"SNES 60.099 @ 119.961", 119.961, 60.0988}, {"SNES 60.099 @ 144", 144.000, 60.0988},
      {"SNES 60.099 @ 165", 165.000, 60.0988},     {"SNES 60.099 @ 240", 240.000, 60.0988},
      {"GBA  59.727 @ 60.000", 60.000, 59.7275},   {"PAL  50.007 @ 59.959", 59.959, 50.007},
      {"PAL  50.007 @ 100", 100.000, 50.007},
  };

  std::printf("%.0f seconds per run, +/-%.0f%% scheduling noise on the clock policies.\n\n", RUN_SECONDS,
              CLOCK_NOISE * 100);

  for (const auto &scenario : scenarios) {
    std::printf("%s\n", scenario.name);
    report("clock @ content rate", runClock(scenario.displayHz, scenario.contentFps, 1));

    const auto k = refreshesPerFrame(scenario.displayHz, scenario.contentFps);
    const auto locked = scenario.displayHz / k;

    if (std::fabs(1.0 - scenario.contentFps / locked) > 0.05) {
      std::printf("  %-24s not matchable (k=%d -> %.3ffps)\n", "clock @ display/k", k, locked);
      std::printf("\n");
      continue;
    }

    report("clock @ display/k", runClock(scenario.displayHz, locked, 1));
    report("pass counting", runPassCounting(scenario.displayHz, k));
    std::printf("\n");
  }

  return 0;
}

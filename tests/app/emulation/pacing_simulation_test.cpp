// TODO: NEEDS REVIEW
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <emulation/frame_pacer.hpp>
#include <gtest/gtest.h>
#include <vector>

using firelight::emulation::FramePacer;
using firelight::emulation::SyncMode;

namespace {
constexpr int64_t SECOND_NS = 1000000000;
constexpr double SNES_FPS = 60.0988;

// How late the loop wakes past its deadline, which the real waiter spins down to well under this
constexpr int64_t WAKE_LATE_NS = 100000;

// TODO
// How far before its deadline a tick still counts as due, which is the controller's own tolerance
constexpr int64_t EARLY_NS = 100000;

struct World {
  double realHz = 59.959;
  double reportedHz = 60.0;
  double contentFps = 60.0;
  SyncMode mode = SyncMode::Display;
  bool presentationLocked = true;
  /** Present jitter as a fraction of a refresh */
  double jitter = 0.0;
  /** How late the loop wakes past its deadline, at most, in ns; the real waiter averages 0.26 ms */
  int64_t wakeJitterNs = 0;
  /** How long a frame takes the core, from the tick to the picture being ready */
  int64_t frameTimeNs = 1500000;
  /** How far every other frame's time swings around frameTimeNs, peak to peak */
  int64_t frameTimeSwingNs = 0;
  /** How long a pass takes to put the picture on the target */
  int64_t passTimeNs = 500000;
  // TODO
  /** Whether a Fixed clock may be held to the display's grid at all */
  bool shouldHoldFixedClock = true;
  double seconds = 300.0;
};

// TODO
/** A refresh the picture did not advance by exactly one frame at */
struct Hitch {
  int64_t atNs = 0;
  int refreshIndex = 0;
  /** Frames replaced before any refresh showed them, or 0 for a refresh that showed nothing new */
  int lost = 0;
};

struct Outcome {
  int framesRun = 0;
  int framesShown = 0;
  /** Frames the core ran that a newer one replaced before a refresh showed them */
  int neverSeen = 0;
  /** Refreshes after the first two seconds with nothing new to show: a frame shown twice */
  int repeats = 0;
  // TODO
  /** Every hitch in the order they happened, and when each frame ran */
  std::vector<Hitch> hitches;
  std::vector<int64_t> frameAtNs;
};

// TODO
/** How a run's hitches sat against a beat arriving beatHz times a second */
struct BeatFit {
  /** The furthest any hitch strayed from where that beat would have put it, in events */
  double deviation = 0.0;
  int shortestGapRefreshes = 0;
  int longestGapRefreshes = 0;
  int count = 0;
};

BeatFit fitToTheBeat(const std::vector<Hitch> &hitches, const double beatHz) {
  BeatFit fit;
  fit.count = static_cast<int>(hitches.size());

  if (hitches.size() < 2) {
    return fit;
  }

  fit.shortestGapRefreshes = hitches[1].refreshIndex - hitches[0].refreshIndex;
  fit.longestGapRefreshes = fit.shortestGapRefreshes;

  for (size_t index = 1; index < hitches.size(); ++index) {
    const auto gap = hitches[index].refreshIndex - hitches[index - 1].refreshIndex;
    fit.shortestGapRefreshes = std::min(fit.shortestGapRefreshes, gap);
    fit.longestGapRefreshes = std::max(fit.longestGapRefreshes, gap);
    const auto elapsed = static_cast<double>(hitches[index].atNs - hitches[0].atNs) / SECOND_NS;
    fit.deviation = std::max(fit.deviation, std::abs(static_cast<double>(index) - beatHz * elapsed));
  }

  return fit;
}

// TODO
/**
 * How far the game's clock strayed from the content's own rate over the run, in frames: the span of
 * the time elapsed against the frames that ran in it, from the first frame on
 */
double deficitSpan(const std::vector<int64_t> &frameAtNs, const double contentFps) {
  if (frameAtNs.size() < 2) {
    return 0.0;
  }

  auto lowest = 0.0;
  auto highest = 0.0;

  for (size_t index = 0; index < frameAtNs.size(); ++index) {
    const auto elapsed = static_cast<double>(frameAtNs[index] - frameAtNs[0]) / SECOND_NS;
    const auto deficit = elapsed * contentFps - static_cast<double>(index);
    lowest = std::min(lowest, deficit);
    highest = std::max(highest, deficit);
  }

  return highest - lowest;
}

struct Noise {
  uint64_t state = 0x9E3779B97F4A7C15ULL;

  double next() {
    state = state * 6364136223846793005ULL + 1442695040888963407ULL;
    return static_cast<double>(state >> 11) / static_cast<double>(1ULL << 53) * 2.0 - 1.0;
  }
};

// TODO
// Drives the real pacer against a modelled display: the loop wakes at its deadlines, runs the frame
// for as long as the core takes, and every refresh shows the newest picture that was ready by then
Outcome simulate(const World &world) {
  FramePacer pacer;
  pacer.configure({.mode = world.mode,
                   .contentFps = world.contentFps,
                   .displayHz = world.reportedHz,
                   .presentationLocked = world.presentationLocked,
                   .shouldHoldFixedClock = world.shouldHoldFixedClock});
  pacer.setReady(true);
  pacer.notePassDuration(world.passTimeNs);

  Outcome outcome;
  Noise noise;
  const auto refreshNs = SECOND_NS / world.realHz;
  const auto endNs = SECOND_NS + static_cast<int64_t>(world.seconds * SECOND_NS);
  auto nextRefreshNs = static_cast<double>(SECOND_NS);
  auto nowNs = SECOND_NS;
  auto busyUntilNs = SECOND_NS;
  auto published = 0;
  auto shown = 0;
  auto readyAtNs = int64_t(0);
  auto passRequested = true;
  auto slowFrame = false;
  auto refreshIndex = 0;

  while (nowNs < endNs) {
    const auto deadlineNs = pacer.getNextDeadlineNs();
    const auto wakeLateNs =
        WAKE_LATE_NS + static_cast<int64_t>((noise.next() + 1.0) * 0.5 * static_cast<double>(world.wakeJitterNs));
    const auto nextTickNs =
        deadlineNs > 0 ? std::max({deadlineNs + wakeLateNs, busyUntilNs, nowNs + 1}) : nowNs + SECOND_NS / 1000;
    const auto refreshAt = static_cast<int64_t>(nextRefreshNs);

    if (refreshAt <= nextTickNs) {
      // TODO
      // A pass asked for since the last refresh presents at this one, with the newest picture that
      // was ready in time, or the one already showing when none was
      const auto newest = readyAtNs <= refreshAt ? published : published - 1;

      if (newest > shown || passRequested) {
        pacer.noteSubmit(refreshAt + static_cast<int64_t>(noise.next() * world.jitter * refreshNs));
      }

      // TODO
      // Frames the display never got to, and refreshes it had nothing new for, are both hitches; the
      // first stretch is left out of both so a run is judged once it has settled
      const auto isCounting = nowNs > SECOND_NS * 3;

      if (newest > shown) {
        outcome.framesShown++;

        if (newest - shown - 1 > 0 && isCounting) {
          outcome.neverSeen += newest - shown - 1;
          outcome.hitches.push_back({.atNs = refreshAt, .refreshIndex = refreshIndex, .lost = newest - shown - 1});
        }

        shown = newest;
      } else if (isCounting) {
        outcome.repeats++;
        outcome.hitches.push_back({.atNs = refreshAt, .refreshIndex = refreshIndex, .lost = 0});
      }

      passRequested = false;
      nextRefreshNs += refreshNs;
      refreshIndex++;
      nowNs = refreshAt;
      continue;
    }

    nowNs = nextTickNs;

    if (deadlineNs > 0) {
      pacer.noteWakeLateness(nowNs - deadlineNs);
    }

    const auto decision = pacer.tick(nowNs);

    if (decision.shouldRequestRender || decision.framesToRun > 0) {
      passRequested = true;
    }

    for (auto frame = 0; frame < decision.framesToRun; ++frame) {
      const auto frameTimeNs =
          world.frameTimeNs + (slowFrame ? world.frameTimeSwingNs / 2 : -world.frameTimeSwingNs / 2);
      slowFrame = !slowFrame;
      outcome.framesRun++;
      outcome.frameAtNs.push_back(nowNs);
      published++;
      busyUntilNs = nowNs + frameTimeNs;
      readyAtNs = busyUntilNs;
      pacer.noteFrameDuration(frameTimeNs);
    }
  }

  return outcome;
}
} // namespace

// The doc's table: a clock at the reported 60.000 against a panel really at 59.959 runs 0.068 % fast,
// which over five minutes is a dozen frames that were replaced before a refresh showed them
TEST(PacingSimulationTest, AClockAloneLosesFramesToAMisreportedRefreshRate) {
  const auto outcome = simulate({.presentationLocked = false});

  EXPECT_GE(outcome.neverSeen, 8);
  EXPECT_LE(outcome.neverSeen, 16);
}

TEST(PacingSimulationTest, ThePhaseLockLosesNothingToAMisreportedRefreshRate) {
  const auto outcome = simulate({});

  EXPECT_EQ(outcome.neverSeen, 0);
  EXPECT_EQ(outcome.repeats, 0);
  EXPECT_NEAR(outcome.framesRun, 300.0 * 59.959, 3.0);
}

TEST(PacingSimulationTest, ThePhaseLockHoldsThroughPresentJitter) {
  for (const auto jitter : {0.01, 0.05}) {
    const auto outcome = simulate({.jitter = jitter});

    EXPECT_EQ(outcome.neverSeen, 0) << "at " << jitter * 100 << " % jitter";
    EXPECT_EQ(outcome.repeats, 0) << "at " << jitter * 100 << " % jitter";
  }
}

TEST(PacingSimulationTest, ThePhaseLockHoldsUnderWakeJitter) {
  const auto outcome = simulate({.wakeJitterNs = 1000000});

  EXPECT_EQ(outcome.neverSeen, 0);
  EXPECT_EQ(outcome.repeats, 0);
}

TEST(PacingSimulationTest, ThePhaseLockHoldsAtTwoRefreshesPerFrame) {
  const auto outcome = simulate({.realHz = 119.961, .reportedHz = 120.0, .jitter = 0.01});

  EXPECT_EQ(outcome.neverSeen, 0);
  EXPECT_NEAR(outcome.framesRun, 300.0 * 119.961 / 2.0, 3.0);
}

// TODO
// The rate difference is paid on the beat and nowhere else: the hitches arrive where a beat of
// |content - display| puts them, at the gap that rate implies, however heavy and uneven the frames
TEST(PacingSimulationTest, NativeHitchesOnlyOnTheBeat) {
  struct Case {
    const char *name;
    double contentFps;
    double realHz;
    int64_t frameTimeNs;
    int64_t swingNs;
  };

  for (const auto &probe : {Case{"a core faster than the panel", SNES_FPS, 59.959, 1500000, 0},
                            Case{"a core slower than the panel", 59.94, 60.0, 1500000, 0},
                            Case{"a core drawing every other frame", 60.0, 59.959, 10000000, 6000000}}) {
    const auto outcome = simulate({.realHz = probe.realHz,
                                   .contentFps = probe.contentFps,
                                   .mode = SyncMode::Fixed,
                                   .wakeJitterNs = 600000,
                                   .frameTimeNs = probe.frameTimeNs,
                                   .frameTimeSwingNs = probe.swingNs});
    const auto beatHz = std::abs(probe.contentFps - probe.realHz);
    const auto fit = fitToTheBeat(outcome.hitches, beatHz);
    const auto gapRefreshes = probe.realHz / beatHz;

    EXPECT_GE(fit.count, static_cast<int>(beatHz * 290.0)) << probe.name;
    EXPECT_LE(fit.count, static_cast<int>(beatHz * 300.0) + 1) << probe.name;
    EXPECT_LE(fit.deviation, 1.0) << probe.name;
    EXPECT_GE(fit.shortestGapRefreshes, static_cast<int>(gapRefreshes * 0.9)) << probe.name;
    EXPECT_LE(fit.longestGapRefreshes, static_cast<int>(gapRefreshes * 1.1)) << probe.name;
  }
}

// TODO
// The clock keeps the content's rate from its first frame: all that is ever outstanding is the frame
// the beat has not paid yet, however late the loop woke and however uneven the core's frames are
TEST(PacingSimulationTest, NativeLosesNoTimeItDoesNotAdmitTo) {
  for (const auto swingNs : {int64_t(0), int64_t(6000000)}) {
    const auto frameTimeNs = swingNs > 0 ? 10000000 : 1500000;

    for (const auto contentFps : {SNES_FPS, 60.0}) {
      const auto outcome = simulate({.contentFps = contentFps,
                                     .mode = SyncMode::Fixed,
                                     .wakeJitterNs = 600000,
                                     .frameTimeNs = frameTimeNs,
                                     .frameTimeSwingNs = swingNs});
      const auto slackNs = WAKE_LATE_NS + EARLY_NS + 600000 + swingNs;
      const auto owed = 1.0 + static_cast<double>(slackNs) / SECOND_NS * contentFps;

      EXPECT_LE(deficitSpan(outcome.frameAtNs, contentFps), owed)
          << "at " << contentFps << " fps, swing " << swingNs / 1000 << " us";
    }
  }
}

// TODO
// Presents that stray far enough from the grid take the hold away and give it back. What the content
// is owed survives that, so the rate over the run is still the content's
TEST(PacingSimulationTest, NativeKeepsTheRateWhenTheGridComesAndGoes) {
  for (const auto jitter : {0.15, 0.25}) {
    const auto outcome =
        simulate({.contentFps = SNES_FPS, .mode = SyncMode::Fixed, .jitter = jitter, .wakeJitterNs = 600000});
    const auto slackNs = WAKE_LATE_NS + EARLY_NS + 600000;

    // The beat's own frame, the slack, and at most one snap onto the grid not yet paid back
    const auto owed = 2.0 + static_cast<double>(slackNs) / SECOND_NS * SNES_FPS;

    EXPECT_LE(deficitSpan(outcome.frameAtNs, SNES_FPS), owed) << "at " << jitter * 100 << " % jitter";
    EXPECT_NEAR(outcome.framesRun, 300.0 * SNES_FPS, 3.0) << "at " << jitter * 100 << " % jitter";
  }
}

// TODO
// A clock the display's grid has no hold over owes nothing at any point, whatever the core does, and
// pays the rate difference as a pulldown: with steady frames the same two gaps over and over
TEST(PacingSimulationTest, AnUnheldClockKeepsExactTimeAndPullsDown) {
  const auto steady = simulate({.contentFps = 50.0, .mode = SyncMode::Fixed, .wakeJitterNs = 600000});
  const auto fit = fitToTheBeat(steady.hitches, 59.959 - 50.0);

  EXPECT_LE(deficitSpan(steady.frameAtNs, 50.0), 0.1);
  EXPECT_GE(fit.shortestGapRefreshes, 5);
  EXPECT_LE(fit.longestGapRefreshes, 7);

  // TODO
  // A core whose frames swing further than they drift against the grid lands either side of the pass
  // and the pulldown breaks up, which hardware agrees with; the rate it runs at does not move
  const auto uneven = simulate({.contentFps = 50.0,
                                .mode = SyncMode::Fixed,
                                .wakeJitterNs = 600000,
                                .frameTimeNs = 10000000,
                                .frameTimeSwingNs = 6000000,
                                .shouldHoldFixedClock = false});

  EXPECT_LE(deficitSpan(uneven.frameAtNs, 50.0), 0.1);
}

TEST(PacingSimulationTest, AnExactMatchLosesNothing) {
  const auto outcome = simulate({.realHz = 60.0, .mode = SyncMode::Fixed, .presentationLocked = false});

  EXPECT_LE(outcome.neverSeen, 1);
}

// TODO
// The margin is what earns the beat: the loop wakes late by up to almost all of it, on frames heavy
// enough for it to decide anything, and the hitches still arrive only where the beat puts them
TEST(PacingSimulationTest, NativeStaysAtTheBeatUnderWakeJitter) {
  for (const auto jitterNs : {1000000LL, 2000000LL, 3500000LL}) {
    const auto outcome = simulate({.contentFps = SNES_FPS,
                                   .mode = SyncMode::Fixed,
                                   .wakeJitterNs = jitterNs,
                                   .frameTimeNs = 10000000,
                                   .frameTimeSwingNs = 6000000});
    const auto fit = fitToTheBeat(outcome.hitches, SNES_FPS - 59.959);

    const auto slackNs = WAKE_LATE_NS + EARLY_NS + jitterNs + 6000000;
    const auto owed = 1.0 + static_cast<double>(slackNs) / SECOND_NS * SNES_FPS;

    EXPECT_LE(fit.deviation, 1.0) << "at " << jitterNs / 1000 << " us";
    EXPECT_GE(fit.shortestGapRefreshes, 385) << "at " << jitterNs / 1000 << " us";
    EXPECT_LE(deficitSpan(outcome.frameAtNs, SNES_FPS), owed) << "at " << jitterNs / 1000 << " us";
  }
}

// A core slower than the panel pays the beat the other way, one repeat every 16.7 s
TEST(PacingSimulationTest, ASlowerCoreStaysAtItsBeatUnderWakeJitter) {
  const auto outcome =
      simulate({.realHz = 60.0, .contentFps = 59.94, .mode = SyncMode::Fixed, .wakeJitterNs = 1000000});

  EXPECT_LE(outcome.neverSeen, 2);
  EXPECT_GE(outcome.repeats, 12);
  EXPECT_LE(outcome.repeats, 24);
  EXPECT_NEAR(outcome.framesRun, 300.0 * 59.94, 3.0);
}

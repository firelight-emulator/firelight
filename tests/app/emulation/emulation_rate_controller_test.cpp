// TODO: NEEDS REVIEW
#include <algorithm>
#include <cmath>
#include <emulation/emulation_rate_controller.hpp>
#include <gtest/gtest.h>
#include <vector>

namespace firelight::emulation {

namespace {
constexpr int64_t SECOND_NS = 1000000000LL;
constexpr double SNES_FPS = 60.0988;

EmulationRateController fixedAt(const double fps) {
  EmulationRateController controller;
  controller.configure({.mode = SyncMode::Fixed, .contentFps = fps});
  return controller;
}

EmulationRateController displayOf(const double contentFps, const double displayHz,
                                  const bool presentationLocked = true) {
  EmulationRateController controller;
  controller.configure({.mode = SyncMode::Display,
                        .contentFps = contentFps,
                        .displayHz = displayHz,
                        .presentationLocked = presentationLocked});
  return controller;
}

EmulationRateController autoOf(const double contentFps, const double displayHz, const bool presentationLocked) {
  EmulationRateController controller;
  controller.configure({.mode = SyncMode::Auto,
                        .contentFps = contentFps,
                        .displayHz = displayHz,
                        .presentationLocked = presentationLocked});
  return controller;
}

// Frames produced over `seconds` of play, asked at `stepNs` intervals the way a wait loop would
int runFor(EmulationRateController &controller, const double seconds, const int64_t stepNs) {
  auto now = SECOND_NS;
  auto frames = controller.framesDue(now);

  for (auto elapsed = int64_t(0); elapsed < static_cast<int64_t>(seconds * SECOND_NS); elapsed += stepNs) {
    now += stepNs;
    frames += controller.framesDue(now);
  }

  return frames;
}

// What a run against the grid saw: the anchor's phase after each frame, and how the anchor stepped
struct GridRun {
  std::vector<double> phases;
  /** Ticks that did not move the anchor: a frame run twice in one refresh */
  int doubles = 0;
  /** Ticks that moved the anchor two refreshes: a refresh with nothing new */
  int repeats = 0;
  /** Simulated time from the first deadline to the last, in ns */
  int64_t elapsedNs = 0;
};

// Runs a held controller on its own deadlines against presents on a fixed refresh grid, feeding it
// the frame time given, and counts the slips by how the anchor stepped
GridRun runHeld(EmulationRateController &controller, const int64_t gridStartNs, const int64_t refreshNs,
                const int frames, const int64_t frameTimeNs = 1500000, const int64_t frameTimeSwingNs = 0) {
  GridRun run;
  auto nextPresentNs = gridStartNs;
  auto nowNs = gridStartNs + refreshNs / 3;
  controller.framesDue(nowNs);
  const auto firstDeadlineNs = controller.getNextDeadlineNs();
  auto slowFrame = false;

  for (auto frame = 0; frame < frames; ++frame) {
    const auto deadlineNs = controller.getNextDeadlineNs();

    while (nextPresentNs <= deadlineNs) {
      controller.notePresent(nextPresentNs);
      nextPresentNs += refreshNs;
    }

    nowNs = std::max(deadlineNs, nowNs);
    controller.framesDue(nowNs);
    const auto anchorNs = controller.getNextDeadlineNs();
    const auto stepNs = anchorNs - deadlineNs;

    // The grid locks after thirty presents and the anchor is put onto the target once; only the
    // steps after that say anything about slips
    if (frame >= 100 && stepNs < refreshNs / 2) {
      run.doubles++;
    } else if (frame >= 100 && stepNs > refreshNs * 3 / 2) {
      run.repeats++;
    }

    const auto thisFrameNs = frameTimeNs + (slowFrame ? frameTimeSwingNs / 2 : -frameTimeSwingNs / 2);
    slowFrame = !slowFrame;
    controller.noteFrameDuration(thisFrameNs);
    nowNs += thisFrameNs;
    const auto sinceGrid = ((anchorNs - gridStartNs) % refreshNs + refreshNs) % refreshNs;
    run.phases.push_back(static_cast<double>(sinceGrid) / static_cast<double>(refreshNs));
    run.elapsedNs = anchorNs - firstDeadlineNs;
  }

  return run;
}

EmulationRateController heldNativeOf(const double contentFps, const double displayHz) {
  EmulationRateController controller;
  controller.configure(
      {.mode = SyncMode::Fixed, .contentFps = contentFps, .displayHz = displayHz, .presentationLocked = true});
  return controller;
}

// Runs the controller on its own deadlines against presents on a fixed refresh grid, returning the
// phase of each frame's anchor inside the refresh that precedes it
std::vector<double> runAgainstGrid(EmulationRateController &controller, const int64_t gridStartNs,
                                   const int64_t refreshNs, const int frames) {
  std::vector<double> phases;
  auto nextPresentNs = gridStartNs;
  auto nowNs = gridStartNs + refreshNs / 3;
  controller.framesDue(nowNs);

  for (auto frame = 0; frame < frames; ++frame) {
    const auto deadlineNs = controller.getNextDeadlineNs();

    while (nextPresentNs <= deadlineNs) {
      controller.notePresent(nextPresentNs);
      nextPresentNs += refreshNs;
    }

    nowNs = deadlineNs;
    controller.framesDue(nowNs);
    const auto anchorNs = controller.getNextDeadlineNs();
    const auto sinceGrid = (anchorNs - gridStartNs) % refreshNs;
    phases.push_back(static_cast<double>(sinceGrid) / static_cast<double>(refreshNs));
  }

  return phases;
}
} // namespace

//****************
// Fixed
//****************

TEST(EmulationRateControllerTest, FixedRunsOneFramePerInterval) {
  auto controller = fixedAt(60.0);
  const auto intervalNs = SECOND_NS / 60;

  auto now = SECOND_NS;
  EXPECT_EQ(controller.framesDue(now), 1);

  now += intervalNs;
  EXPECT_EQ(controller.framesDue(now), 1);

  now += intervalNs;
  EXPECT_EQ(controller.framesDue(now), 1);
}

TEST(EmulationRateControllerTest, FixedRunsNothingBeforeTheFrameIsDue) {
  auto controller = fixedAt(60.0);

  auto now = SECOND_NS;
  controller.framesDue(now);

  now += SECOND_NS / 120;
  EXPECT_EQ(controller.framesDue(now), 0);
}

TEST(EmulationRateControllerTest, FixedHoldsAnAwkwardRateOverAMinute) {
  auto controller = fixedAt(SNES_FPS);

  const auto frames = runFor(controller, 60.0, SECOND_NS / 1000);

  EXPECT_NEAR(frames, 60.0 * SNES_FPS, 2.0);
}

TEST(EmulationRateControllerTest, FixedHoldsANonStandardRate) {
  auto controller = fixedAt(50.0);

  const auto frames = runFor(controller, 10.0, SECOND_NS / 1000);

  EXPECT_NEAR(frames, 500.0, 2.0);
}

TEST(EmulationRateControllerTest, NeverReturnsMoreThanOneFrame) {
  auto controller = fixedAt(60.0);
  auto now = SECOND_NS;
  uint64_t state = 12345;

  for (auto step = 0; step < 5000; ++step) {
    state = state * 6364136223846793005ULL + 1442695040888963407ULL;
    // Anything from a hair early to five frames late
    now += static_cast<int64_t>((state >> 40) % (5 * SECOND_NS / 60));
    EXPECT_LE(controller.framesDue(now), 1) << "at step " << step;
  }
}

//****************
// The anchor schedule
//****************

TEST(EmulationRateControllerTest, ALongStallDoesNotReplayAsFastForward) {
  auto controller = fixedAt(60.0);

  auto now = SECOND_NS;
  controller.framesDue(now);

  now += 10 * SECOND_NS;
  EXPECT_EQ(controller.framesDue(now), 1);
  EXPECT_EQ(controller.framesDue(now + SECOND_NS / 120), 0);
}

TEST(EmulationRateControllerTest, LateByLessThanAPeriodRunsNowAndKeepsTheGrid) {
  auto controller = fixedAt(60.0);
  const auto intervalNs = SECOND_NS / 60;

  const auto start = SECOND_NS;
  controller.framesDue(start);
  ASSERT_EQ(controller.getNextDeadlineNs(), start + intervalNs);

  // Half a frame late
  EXPECT_EQ(controller.framesDue(start + intervalNs + intervalNs / 2), 1);
  EXPECT_EQ(controller.getNextDeadlineNs(), start + 2 * intervalNs) << "the grid moved with the late frame";
}

TEST(EmulationRateControllerTest, LateByLessThanTwoPeriodsCatchesUpWithOneFrame) {
  auto controller = fixedAt(60.0);
  const auto intervalNs = SECOND_NS / 60;

  const auto start = SECOND_NS;
  controller.framesDue(start);

  // A frame and a half late: this frame runs now, the one owed runs half a period on, and the grid
  // is where it was
  const auto lateNs = start + intervalNs + intervalNs + intervalNs / 2;
  EXPECT_EQ(controller.framesDue(lateNs), 1);
  EXPECT_EQ(controller.getNextDeadlineNs(), start + 2 * intervalNs);
  EXPECT_EQ(controller.framesDue(lateNs + intervalNs / 2), 1);
  EXPECT_EQ(controller.getNextDeadlineNs(), start + 3 * intervalNs);
}

TEST(EmulationRateControllerTest, LateByMoreThanTwoPeriodsReanchors) {
  auto controller = fixedAt(60.0);
  const auto intervalNs = SECOND_NS / 60;

  const auto start = SECOND_NS;
  controller.framesDue(start);

  // Two and a half frames late: one frame runs, the rest of the debt is dropped
  const auto lateNs = start + 3 * intervalNs + intervalNs / 2;
  EXPECT_EQ(controller.framesDue(lateNs), 1);
  EXPECT_EQ(controller.getNextDeadlineNs(), lateNs + intervalNs);
  EXPECT_EQ(controller.framesDue(lateNs + intervalNs / 2), 0) << "the dropped frames came back";
}

TEST(EmulationRateControllerTest, AHairEarlyStillCounts) {
  auto controller = fixedAt(60.0);
  const auto intervalNs = SECOND_NS / 60;

  const auto start = SECOND_NS;
  controller.framesDue(start);

  EXPECT_EQ(controller.framesDue(start + intervalNs - 50000), 1);
}

TEST(EmulationRateControllerTest, ResetDropsThePhase) {
  auto controller = fixedAt(60.0);

  auto now = SECOND_NS;
  controller.framesDue(now);
  now += SECOND_NS / 90;
  controller.framesDue(now);

  controller.reset();
  EXPECT_EQ(controller.getNextDeadlineNs(), 0);

  // The first call after a reset starts a fresh cadence rather than owing the gap
  now += 10 * SECOND_NS;
  EXPECT_EQ(controller.framesDue(now), 1);
}

TEST(EmulationRateControllerTest, TheDeadlineIsOneIntervalAfterTheLastFrame) {
  auto controller = fixedAt(60.0);

  const auto now = SECOND_NS;
  controller.framesDue(now);

  EXPECT_NEAR(static_cast<double>(controller.getNextDeadlineNs() - now), static_cast<double>(SECOND_NS) / 60.0,
              1000000.0);
}

//****************
// Display
//****************

TEST(EmulationRateControllerTest, DisplayRunsEveryRefreshWhenTheRatesMatch) {
  auto controller = displayOf(60.0, 60.0);

  EXPECT_EQ(controller.getResolvedMode(), SyncMode::Display);
  EXPECT_EQ(controller.getRefreshesPerFrame(), 1);
  EXPECT_NEAR(runFor(controller, 10.0, SECOND_NS / 1000), 600.0, 2.0);
}

TEST(EmulationRateControllerTest, DisplayHoldsAFrameForTwoRefreshesAt120For60) {
  auto controller = displayOf(60.0, 120.0);

  EXPECT_EQ(controller.getRefreshesPerFrame(), 2);
  EXPECT_NEAR(controller.getEffectiveFps(), 60.0, 0.001);

  controller.framesDue(SECOND_NS);
  EXPECT_NEAR(static_cast<double>(controller.getNextDeadlineNs() - SECOND_NS), 2.0 * SECOND_NS / 120.0, 1000.0);
  EXPECT_NEAR(runFor(controller, 10.0, SECOND_NS / 1000), 600.0, 2.0);
}

// A 60 Hz panel can't show 60.0988, so the game runs at the panel's rate and the audio is stretched
// to match — the same bargain the old monitor mode struck
TEST(EmulationRateControllerTest, DisplayBendsAudioWhenItRoundsTheContentRate) {
  auto controller = displayOf(SNES_FPS, 60.0);

  EXPECT_EQ(controller.getRefreshesPerFrame(), 1);
  EXPECT_NEAR(controller.getEffectiveFps(), 60.0, 0.001);
  EXPECT_NEAR(controller.getAudioRatio(), 60.0 / SNES_FPS, 0.0001);
}

// TODO
// Named outright, Display takes the nearest division whatever it costs: 48 fps on 120 Hz is every
// third refresh at 40 fps, a sixth slow, because that is what the mode says it does
TEST(EmulationRateControllerTest, DisplayTakesTheNearestDivisionHoweverFarOutItIs) {
  auto controller = displayOf(48.0, 120.0);

  EXPECT_EQ(controller.getResolvedMode(), SyncMode::Display);
  EXPECT_EQ(controller.getRefreshesPerFrame(), 3);
  EXPECT_NEAR(controller.getEffectiveFps(), 40.0, 0.001);
  EXPECT_NEAR(controller.getAudioRatio(), 40.0 / 48.0, 0.0001);
}

// 174.962 / 60.0988 is 2.91, so three refreshes at 58.32 fps: 3 % slow, which Auto refuses and an
// explicit monitor request takes
TEST(EmulationRateControllerTest, DisplayAcceptsAHighRefreshPanelAtItsNearestDivision) {
  auto controller = displayOf(SNES_FPS, 174.962);

  EXPECT_EQ(controller.getResolvedMode(), SyncMode::Display);
  EXPECT_EQ(controller.getRefreshesPerFrame(), 3);
  EXPECT_NEAR(controller.getEffectiveFps(), 174.962 / 3.0, 0.001);

  auto chooser = autoOf(SNES_FPS, 174.962, true);
  EXPECT_EQ(chooser.getResolvedMode(), SyncMode::Fixed);
  EXPECT_NEAR(chooser.getEffectiveFps(), SNES_FPS, 0.001);
}

TEST(EmulationRateControllerTest, DisplayIgnoresAnUnknownRefreshRate) {
  auto controller = displayOf(60.0, 0.0);

  EXPECT_EQ(controller.getResolvedMode(), SyncMode::Fixed);
  EXPECT_EQ(controller.getRefreshesPerFrame(), 0);
  EXPECT_NEAR(controller.getEffectiveFps(), 60.0, 0.001);
}

// Presents that don't wait for the display say nothing about its phase, so the rate is still the
// display's but nothing holds the anchor to it
TEST(EmulationRateControllerTest, DisplayKeepsTheRateButNotThePhaseWhenPresentationDoesNotWait) {
  auto controller = displayOf(SNES_FPS, 60.0, false);

  EXPECT_EQ(controller.getResolvedMode(), SyncMode::Display);
  EXPECT_FALSE(controller.isPhaseLocked());
  EXPECT_EQ(controller.getRefreshesPerFrame(), 1);
  EXPECT_NEAR(controller.getEffectiveFps(), 60.0, 0.001);
  EXPECT_NEAR(controller.getAudioRatio(), 60.0 / SNES_FPS, 0.0001);
  EXPECT_NEAR(runFor(controller, 10.0, SECOND_NS / 1000), 600.0, 2.0);
}

TEST(EmulationRateControllerTest, PresentsNeverMakeAFrameDue) {
  auto controller = displayOf(60.0, 120.0);
  const auto refreshNs = SECOND_NS / 120;

  controller.framesDue(SECOND_NS);
  const auto deadlineNs = controller.getNextDeadlineNs();

  for (auto i = 0; i < 10; ++i) {
    controller.notePresent(SECOND_NS + i * refreshNs);
  }

  EXPECT_EQ(controller.framesDue(SECOND_NS + refreshNs), 0);
  EXPECT_EQ(controller.getNextDeadlineNs(), deadlineNs);
}

TEST(EmulationRateControllerTest, ChangingTheRefreshRateRelocks) {
  auto controller = displayOf(60.0, 60.0);
  ASSERT_EQ(controller.getRefreshesPerFrame(), 1);

  controller.configure({.mode = SyncMode::Display, .contentFps = 60.0, .displayHz = 120.0, .presentationLocked = true});

  EXPECT_EQ(controller.getRefreshesPerFrame(), 2);
}

//****************
// The phase term
//****************

TEST(EmulationRateControllerTest, PhaseCorrectionPullsTheAnchorTowardTheTarget) {
  auto controller = displayOf(60.0, 60.0);
  const auto refreshNs = SECOND_NS / 60;

  const auto phases = runAgainstGrid(controller, SECOND_NS, refreshNs, 400);

  EXPECT_NEAR(phases.back(), EmulationRateController::PHASE_TARGET, 0.02);

  // The first frames after the grid locks are put straight onto the target; from there on the
  // anchor only ever moves by the nudge
  for (size_t index = 60; index < phases.size(); ++index) {
    auto step = std::abs(phases[index] - phases[index - 1]);

    if (step > 0.5) {
      step = 1.0 - step;
    }

    EXPECT_LE(step, EmulationRateController::MAX_NUDGE + 1e-6) << "at frame " << index;
  }
}

TEST(EmulationRateControllerTest, PhaseCorrectionHoldsAtTwoRefreshesPerFrame) {
  auto controller = displayOf(60.0, 120.0);
  const auto refreshNs = SECOND_NS / 120;

  const auto phases = runAgainstGrid(controller, SECOND_NS, refreshNs, 400);

  EXPECT_NEAR(phases.back(), EmulationRateController::PHASE_TARGET, 0.02);
}

TEST(EmulationRateControllerTest, NoPhaseCorrectionWhenPresentationDoesNotWait) {
  auto controller = displayOf(60.0, 60.0, false);
  const auto refreshNs = SECOND_NS / 60;

  const auto phases = runAgainstGrid(controller, SECOND_NS, refreshNs, 200);

  EXPECT_NEAR(phases.back(), phases.front(), 1e-6);
}

TEST(EmulationRateControllerTest, NoPhaseCorrectionWithoutADisplay) {
  auto controller = fixedAt(60.0);
  const auto refreshNs = SECOND_NS / 60;

  const auto phases = runAgainstGrid(controller, SECOND_NS, refreshNs, 200);

  EXPECT_NEAR(phases.back(), phases.front(), 1e-6);
}

// A real panel runs at 59.959 while reporting 60; held to its presents, the game follows the real
// rate and no frame is ever a whole refresh out
TEST(EmulationRateControllerTest, PhaseLockFollowsTheRealRefreshRateNotTheReportedOne) {
  auto controller = displayOf(60.0, 60.0);
  const auto realRefreshNs = static_cast<int64_t>(SECOND_NS / 59.959);

  const auto phases = runAgainstGrid(controller, SECOND_NS, realRefreshNs, 3600);

  for (size_t index = 600; index < phases.size(); ++index) {
    EXPECT_NEAR(phases[index], EmulationRateController::PHASE_TARGET, 0.05) << "at frame " << index;
  }
}

//****************
// Audio
//****************

TEST(EmulationRateControllerTest, AudioRunsAFrameWhenTheSinkHasRoom) {
  EmulationRateController controller;
  controller.configure({.mode = SyncMode::Audio, .contentFps = 60.0});

  controller.setAudioBufferLevel(0.2F);
  EXPECT_EQ(controller.framesDue(SECOND_NS), 1);
}

TEST(EmulationRateControllerTest, AudioRunsNothingWhenTheSinkIsFull) {
  EmulationRateController controller;
  controller.configure({.mode = SyncMode::Audio, .contentFps = 60.0});

  controller.setAudioBufferLevel(0.8F);
  EXPECT_EQ(controller.framesDue(SECOND_NS), 0);
  EXPECT_EQ(controller.framesDue(SECOND_NS + SECOND_NS), 0);
}

TEST(EmulationRateControllerTest, AudioKeepsAnsweringWhileTheSinkIsDraining) {
  EmulationRateController controller;
  controller.configure({.mode = SyncMode::Audio, .contentFps = 60.0});

  controller.setAudioBufferLevel(0.4F);
  EXPECT_EQ(controller.framesDue(SECOND_NS), 1);
  EXPECT_EQ(controller.framesDue(SECOND_NS + 1000), 1);
  EXPECT_EQ(controller.framesDue(SECOND_NS + 2000), 1);
}

TEST(EmulationRateControllerTest, AudioIgnoresTheClock) {
  EmulationRateController controller;
  controller.configure({.mode = SyncMode::Audio, .contentFps = 60.0});

  EXPECT_EQ(controller.getNextDeadlineNs(), 0);
  controller.setAudioBufferLevel(0.9F);
  EXPECT_EQ(controller.framesDue(SECOND_NS), 0);
  EXPECT_EQ(controller.framesDue(SECOND_NS + 10 * SECOND_NS), 0);
}

TEST(EmulationRateControllerTest, AudioFallsBackToTheClockWhenTheSinkCannotBeRead) {
  EmulationRateController controller;
  controller.configure({.mode = SyncMode::Audio, .contentFps = 60.0});

  controller.setAudioBufferLevel(-1.0F);
  EXPECT_NEAR(runFor(controller, 10.0, SECOND_NS / 1000), 600.0, 2.0);
}

//****************
// Auto
//****************

// TODO
// The rule, and the reason for it. Pacing off the sink cannot put frames out evenly: the device only
// reports what it has consumed in whole periods — 10.67 ms, measured — so the gate can only fire on
// those boundaries and frames leave 10.7 or 21.3 ms apart, never the 16.67 they are due. Auto never
// chooses it, however available the sink is; audio is made to fit a clock instead, by rate control
TEST(EmulationRateControllerTest, AutoNeverPacesOffTheSink) {
  struct Case {
    double contentFps;
    double displayHz;
    bool presentationLocked;
  };

  for (const auto &c : {Case{SNES_FPS, 60.0, false}, Case{SNES_FPS, 60.0, true}, Case{60.0, 120.0, true},
                        Case{48.0, 120.0, true}, Case{60.0, 0.0, false}}) {
    auto controller = autoOf(c.contentFps, c.displayHz, c.presentationLocked);

    EXPECT_NE(controller.getResolvedMode(), SyncMode::Audio)
        << c.contentFps << "fps on " << c.displayHz << "Hz, locked=" << c.presentationLocked;
  }
}

TEST(EmulationRateControllerTest, AutoFollowsTheDisplayWhenTheRatesAreClose) {
  auto controller = autoOf(SNES_FPS, 60.0, true);

  EXPECT_EQ(controller.getResolvedMode(), SyncMode::Display);
  EXPECT_TRUE(controller.isPhaseLocked());
  EXPECT_EQ(controller.getRefreshesPerFrame(), 1);
  EXPECT_NEAR(controller.getEffectiveFps(), 60.0, 0.001);
  EXPECT_NEAR(controller.getAudioRatio(), 60.0 / SNES_FPS, 0.0001);
}

TEST(EmulationRateControllerTest, AutoHoldsAFrameForTwoRefreshesOnAHighRefreshPanel) {
  auto controller = autoOf(60.0, 120.0, true);

  EXPECT_EQ(controller.getResolvedMode(), SyncMode::Display);
  EXPECT_EQ(controller.getRefreshesPerFrame(), 2);
  EXPECT_NEAR(runFor(controller, 10.0, SECOND_NS / 1000), 600.0, 2.0);
}

// Unlocked, a present says nothing about the display's phase, so the rate is kept and the phase is
// left alone
TEST(EmulationRateControllerTest, AutoKeepsTheDisplayRateWithoutThePhaseWhenPresentationDoesNotWait) {
  auto controller = autoOf(SNES_FPS, 60.0, false);

  EXPECT_EQ(controller.getResolvedMode(), SyncMode::Display);
  EXPECT_FALSE(controller.isPhaseLocked());
  EXPECT_NEAR(controller.getEffectiveFps(), 60.0, 0.001);
  EXPECT_NEAR(controller.getAudioRatio(), 60.0 / SNES_FPS, 0.0001);
  EXPECT_NEAR(runFor(controller, 10.0, SECOND_NS / 1000), 600.0, 2.0);
}

// 120/48 is 2.5, so the nearest division is a sixth out, far past what can be heard
TEST(EmulationRateControllerTest, AutoFallsBackToTheContentRateWhenNothingFits) {
  auto controller = autoOf(48.0, 120.0, true);

  EXPECT_EQ(controller.getResolvedMode(), SyncMode::Fixed);
  EXPECT_EQ(controller.getRefreshesPerFrame(), 0);
  EXPECT_NEAR(controller.getEffectiveFps(), 48.0, 0.001);
  EXPECT_NEAR(controller.getAudioRatio(), 1.0, 0.0001);
  EXPECT_NEAR(runFor(controller, 10.0, SECOND_NS / 1000), 480.0, 2.0);
}

TEST(EmulationRateControllerTest, AutoIgnoresAnUnknownRefreshRate) {
  auto controller = autoOf(60.0, 0.0, true);

  EXPECT_EQ(controller.getResolvedMode(), SyncMode::Fixed);
  EXPECT_NEAR(controller.getEffectiveFps(), 60.0, 0.001);
}

// Turning vsync on or off changes whether the phase can be held, and nothing else
TEST(EmulationRateControllerTest, PresentationLockOnlyChangesWhetherThePhaseIsHeld) {
  auto controller = autoOf(60.0, 60.0, false);
  ASSERT_EQ(controller.getResolvedMode(), SyncMode::Display);
  ASSERT_FALSE(controller.isPhaseLocked());

  controller.framesDue(SECOND_NS);
  const auto deadlineNs = controller.getNextDeadlineNs();
  ASSERT_NE(deadlineNs, 0);

  controller.configure({.mode = SyncMode::Auto, .contentFps = 60.0, .displayHz = 60.0, .presentationLocked = true});

  EXPECT_EQ(controller.getResolvedMode(), SyncMode::Display);
  EXPECT_TRUE(controller.isPhaseLocked());
  EXPECT_EQ(controller.getNextDeadlineNs(), deadlineNs) << "the cadence did not change, so the anchor stays";
}

// TODO
// Taking the display's rate means stretching the audio by the difference, so what the tolerance has
// to bound is how much of that can be heard rather than how near the two numbers look
TEST(EmulationRateControllerTest, AutoDeclinesADisplayRateFarEnoughOutToBeHeard) {
  // 3.4% out, which is most of a semitone of stretch on everything the game plays
  auto controller = autoOf(58.0, 60.0, false);

  EXPECT_EQ(controller.getResolvedMode(), SyncMode::Fixed);
  EXPECT_NEAR(controller.getEffectiveFps(), 58.0, 0.001);
  EXPECT_NEAR(controller.getAudioRatio(), 1.0, 0.0001);
}

TEST(EmulationRateControllerTest, AutoTakesTheRatesRealContentReports) {
  // NTSC, the SNES, and the Game Boy Advance, all against a 60 Hz panel
  for (const auto contentFps : {59.94, 60.0988, 59.7275}) {
    auto controller = autoOf(contentFps, 60.0, true);

    EXPECT_EQ(controller.getResolvedMode(), SyncMode::Display) << "at " << contentFps;
    EXPECT_NEAR(controller.getEffectiveFps(), 60.0, 0.001) << "at " << contentFps;
  }
}

// A mode named outright is used as named, however well or badly it fits
TEST(EmulationRateControllerTest, AnExplicitModeIsNotSecondGuessed) {
  auto controller = fixedAt(48.0);

  EXPECT_EQ(controller.getResolvedMode(), SyncMode::Fixed);
  EXPECT_NEAR(controller.getEffectiveFps(), 48.0, 0.001);
}

//****************
// Reconfiguring
//****************

// TODO
// A reconfigure lands on a game that is already running, so the frame it has waited most of a period
// for is still due when it was due. Only the phase, which described the old cadence, starts over
TEST(EmulationRateControllerTest, ChangingModeKeepsTheFrameAlreadyDue) {
  EmulationRateController controller;
  controller.configure({.mode = SyncMode::Fixed, .contentFps = 60.0});

  controller.framesDue(SECOND_NS);
  const auto deadlineNs = controller.getNextDeadlineNs();
  ASSERT_NE(deadlineNs, 0);

  controller.configure({.mode = SyncMode::Display, .contentFps = 60.0, .displayHz = 120.0, .presentationLocked = true});

  EXPECT_EQ(controller.getRefreshesPerFrame(), 2);
  EXPECT_EQ(controller.getNextDeadlineNs(), deadlineNs);
}

// TODO
// A rate change moves the frame already due by what the period changed by, so the gap either side of
// the change is one whole period of the rate that is now in force, not a period and a bit of another
TEST(EmulationRateControllerTest, ChangingTheRateMovesTheFrameDueByTheDifference) {
  auto controller = fixedAt(60.0);

  controller.framesDue(SECOND_NS);
  const auto deadlineNs = controller.getNextDeadlineNs();
  ASSERT_NE(deadlineNs, 0);

  controller.configure({.mode = SyncMode::Fixed, .contentFps = 30.0});

  EXPECT_NEAR(controller.getNextDeadlineNs(), deadlineNs + SECOND_NS / 30 - SECOND_NS / 60, 1000);
}

TEST(EmulationRateControllerTest, ReconfiguringWithTheSameCadenceKeepsThePhase) {
  auto controller = fixedAt(60.0);

  controller.framesDue(SECOND_NS);
  const auto deadlineNs = controller.getNextDeadlineNs();
  ASSERT_NE(deadlineNs, 0);

  controller.configure({.mode = SyncMode::Fixed, .contentFps = 60.0});

  EXPECT_EQ(controller.getNextDeadlineNs(), deadlineNs);
}

TEST(EmulationRateControllerTest, ChangingTheDivisionKeepsTheFrameAlreadyDue) {
  auto controller = displayOf(60.0, 120.0);
  ASSERT_EQ(controller.getRefreshesPerFrame(), 2);

  controller.framesDue(SECOND_NS);
  const auto deadlineNs = controller.getNextDeadlineNs();
  ASSERT_NE(deadlineNs, 0);

  // Same 60 fps, but every fourth refresh now rather than every other
  controller.configure({.mode = SyncMode::Display, .contentFps = 60.0, .displayHz = 240.0, .presentationLocked = true});

  EXPECT_EQ(controller.getRefreshesPerFrame(), 4);
  EXPECT_EQ(controller.getNextDeadlineNs(), deadlineNs);
}

// TODO
// Loading, unpausing or seeking leaves a gap that means nothing, so that one really does start over
TEST(EmulationRateControllerTest, AResetDropsTheFrameAlreadyDue) {
  auto controller = fixedAt(60.0);

  controller.framesDue(SECOND_NS);
  ASSERT_NE(controller.getNextDeadlineNs(), 0);

  controller.reset();

  EXPECT_EQ(controller.getNextDeadlineNs(), 0);
}

// TODO
// Once the frame and the pass have been timed, the frame is asked for so that both fit before the
// next refresh with the wake margin to spare
TEST(EmulationRateControllerTest, PhaseTargetFollowsFrameAndPassTime) {
  EmulationRateController controller;
  controller.configure({.mode = SyncMode::Display, .contentFps = 60.0, .displayHz = 60.0, .presentationLocked = true});

  EXPECT_DOUBLE_EQ(controller.getPhaseTarget(), EmulationRateController::PHASE_TARGET);

  controller.noteFrameDuration(4000000);
  controller.notePassDuration(2000000);

  const auto periodNs = 1e9 / 60.0;
  const auto expected = 1.0 - (4000000.0 + 2000000.0 + EmulationRateController::WAKE_MARGIN_NS) / periodNs;
  EXPECT_NEAR(controller.getPhaseTarget(), expected, 0.01);
}

TEST(EmulationRateControllerTest, FrameJitterIsTracked) {
  EmulationRateController controller;
  controller.configure({.mode = SyncMode::Display, .contentFps = 60.0, .displayHz = 60.0, .presentationLocked = true});

  for (auto frame = 0; frame < 64; ++frame) {
    controller.noteFrameDuration(frame % 2 == 0 ? 11000000 : 16000000);
  }

  EXPECT_NEAR(controller.getFrameTimeNs(), 13500000, 1500000);
  EXPECT_NEAR(controller.getSlowestFrameNs(), 16000000, 500000);
  EXPECT_NEAR(controller.getQuickestFrameNs(), 11000000, 500000);
  EXPECT_LT(controller.getSlowSpreadNs(), 500000);
}

// TODO
// A frame that cannot fit before the next present even on its quick days is aimed past the pass of
// the refresh it starts in, so every frame is shown at the refresh after, never half of them
TEST(EmulationRateControllerTest, AFrameThatCannotFitIsAimedPastThePass) {
  EmulationRateController controller;
  controller.configure({.mode = SyncMode::Display, .contentFps = 60.0, .displayHz = 60.0, .presentationLocked = true});
  controller.notePassDuration(500000);

  for (auto frame = 0; frame < 64; ++frame) {
    controller.noteFrameDuration(frame % 2 == 0 ? 12000000 : 17000000);
  }

  EXPECT_LT(controller.getPhaseTarget(), 0.0);
  EXPECT_GE(controller.getPhaseTarget(), EmulationRateController::EARLIEST_PHASE_TARGET);

  // Its quickest frame, 12 ms from an anchor before the present, still lands past the pass
  const auto periodNs = 1e9 / 60.0;
  EXPECT_GT(controller.getPhaseTarget() * periodNs + 12000000.0,
            500000.0 + static_cast<double>(EmulationRateController::WAKE_MARGIN_NS));
}

TEST(EmulationRateControllerTest, PhaseTargetAllowsForFrameJitter) {
  EmulationRateController controller;
  controller.configure({.mode = SyncMode::Display, .contentFps = 60.0, .displayHz = 60.0, .presentationLocked = true});
  controller.notePassDuration(2000000);

  for (auto frame = 0; frame < 64; ++frame) {
    controller.noteFrameDuration(frame % 2 == 0 ? 4000000 : 8000000);
  }

  const auto periodNs = 1e9 / 60.0;
  const auto spentNs =
      static_cast<double>(controller.getSlowestFrameNs() + 2000000 + EmulationRateController::WAKE_MARGIN_NS) +
      EmulationRateController::SLOW_SPREAD_ALLOWANCE * static_cast<double>(controller.getSlowSpreadNs());
  EXPECT_NEAR(controller.getPhaseTarget(), 1.0 - spentNs / periodNs, 0.01);
  EXPECT_LT(controller.getPhaseTarget(), 1.0 - (7500000.0 + 2000000.0 + 1000000.0) / periodNs);
}

//****************
// Held native
//****************

TEST(EmulationRateControllerTest, TheHoldEngagesOnlyWithinTheRateBound) {
  EXPECT_TRUE(heldNativeOf(SNES_FPS, 59.959).isPhaseLocked());
  EXPECT_TRUE(heldNativeOf(60.0, 120.0).isPhaseLocked());
  EXPECT_EQ(heldNativeOf(60.0, 120.0).getHeldRefreshes(), 2);
  EXPECT_TRUE(heldNativeOf(59.94, 60.0).isPhaseLocked());

  // TODO
  // A rate that is not a whole division of the display's moves further against the grid each frame
  // than the timing varies
  EXPECT_FALSE(heldNativeOf(50.0, 59.959).isPhaseLocked());
  EXPECT_FALSE(heldNativeOf(90.0, 60.0).isPhaseLocked());
  EXPECT_FALSE(fixedAt(60.0).isPhaseLocked());

  EmulationRateController unlocked;
  unlocked.configure(
      {.mode = SyncMode::Fixed, .contentFps = SNES_FPS, .displayHz = 59.959, .presentationLocked = false});
  EXPECT_FALSE(unlocked.isPhaseLocked());
  EXPECT_EQ(unlocked.getRefreshesPerFrame(), 0);
  EXPECT_DOUBLE_EQ(unlocked.getAudioRatio(), 1.0);
}

// TODO
// With the hold turned off, native on a display that presentation waits for steps by the content's own
// period on every frame, after the grid has locked as well as before
TEST(EmulationRateControllerTest, NativeWithTheHoldOffStepsByItsOwnPeriod) {
  EmulationRateController controller;
  controller.configure({.mode = SyncMode::Fixed,
                        .contentFps = SNES_FPS,
                        .displayHz = 59.959,
                        .presentationLocked = true,
                        .shouldHoldFixedClock = false});
  const auto refreshNs = static_cast<int64_t>(SECOND_NS / 59.959);
  const auto periodNs = static_cast<int64_t>(SECOND_NS / SNES_FPS);

  EXPECT_FALSE(controller.isPhaseLocked());
  EXPECT_EQ(controller.getHeldRefreshes(), 0);

  auto nextPresentNs = SECOND_NS;
  controller.framesDue(SECOND_NS + refreshNs / 3);

  for (auto frame = 0; frame < 3600; ++frame) {
    const auto deadlineNs = controller.getNextDeadlineNs();

    while (nextPresentNs <= deadlineNs) {
      controller.notePresent(nextPresentNs);
      nextPresentNs += refreshNs;
    }

    EXPECT_EQ(controller.framesDue(deadlineNs), 1);
    EXPECT_EQ(controller.getNextDeadlineNs() - deadlineNs, periodNs) << "at frame " << frame;
  }

  EXPECT_TRUE(controller.getPhase().isLocked());
  EXPECT_EQ(controller.getSlipDebtNs(), 0);
}

TEST(EmulationRateControllerTest, HeldNativeSitsAtTheTargetBetweenSlips) {
  auto controller = heldNativeOf(SNES_FPS, 59.959);
  const auto refreshNs = static_cast<int64_t>(SECOND_NS / 59.959);

  const auto run = runHeld(controller, SECOND_NS, refreshNs, 3000);

  for (size_t index = 100; index < run.phases.size(); ++index) {
    EXPECT_NEAR(run.phases[index], controller.getPhaseTarget(), 0.05) << "at frame " << index;
  }
}

// Snes9x on a 59.959 Hz panel owes a frame every 429 ticks: one double, no repeats, and the frames
// over a minute are the content's own
TEST(EmulationRateControllerTest, HeldNativeSlipsOnceABeat) {
  auto controller = heldNativeOf(SNES_FPS, 59.959);
  const auto refreshNs = static_cast<int64_t>(SECOND_NS / 59.959);

  const auto run = runHeld(controller, SECOND_NS, refreshNs, 3600);

  EXPECT_GE(run.doubles, 7);
  EXPECT_LE(run.doubles, 10);
  EXPECT_EQ(run.repeats, 0);
  EXPECT_NEAR(static_cast<double>(run.elapsedNs) / SECOND_NS, 3600.0 / SNES_FPS, 0.02);
}

TEST(EmulationRateControllerTest, ASlowerCoreRepeatsOnceABeat) {
  auto controller = heldNativeOf(59.94, 60.0);
  const auto refreshNs = SECOND_NS / 60;

  const auto run = runHeld(controller, SECOND_NS, refreshNs, 3600);

  EXPECT_EQ(run.doubles, 0);
  EXPECT_GE(run.repeats, 2);
  EXPECT_LE(run.repeats, 5);
  EXPECT_NEAR(static_cast<double>(run.elapsedNs) / SECOND_NS, 3600.0 / 59.94, 0.02);
}

TEST(EmulationRateControllerTest, AnAlternatingFrameTimeStaysAtTheTarget) {
  auto controller = heldNativeOf(60.0, 59.959);
  const auto refreshNs = static_cast<int64_t>(SECOND_NS / 59.959);

  const auto run = runHeld(controller, SECOND_NS, refreshNs, 3000, 10000000, 6000000);

  EXPECT_LT(controller.getPhaseTarget(), 0.25) << "the slow frame leaves little of the refresh";

  // TODO
  // A target before the present that starts the refresh is the same instant as one late in the
  // refresh before it, which is where the phases are measured
  const auto target = controller.getPhaseTarget();
  const auto targetPhase = target < 0.0 ? target + 1.0 : target;

  for (size_t index = 200; index < run.phases.size(); ++index) {
    EXPECT_NEAR(run.phases[index], targetPhase, 0.05) << "at frame " << index;
  }

  EXPECT_GE(run.doubles, 1);
  EXPECT_LE(run.doubles, 3);
}

TEST(EmulationRateControllerTest, EngagingTheHoldSnapsToTheTarget) {
  auto controller = heldNativeOf(SNES_FPS, 59.959);
  const auto refreshNs = static_cast<int64_t>(SECOND_NS / 59.959);

  // The grid locks after thirty presents; a nudge of half a percent a frame could not have crossed
  // the third of a refresh the anchor started from by frame sixty
  const auto run = runHeld(controller, SECOND_NS, refreshNs, 60);

  EXPECT_NEAR(run.phases.back(), controller.getPhaseTarget(), 0.05);
}

// TODO
// A target pinned at the present itself is measured from the nearer present, so an anchor a hair
// before one is nudged forward onto it rather than pulled back through the whole refresh
TEST(EmulationRateControllerTest, ATargetAtThePresentHoldsThere) {
  auto controller = heldNativeOf(SNES_FPS, 59.959);
  controller.setPhaseTarget(0.0);
  const auto refreshNs = static_cast<int64_t>(SECOND_NS / 59.959);

  const auto run = runHeld(controller, SECOND_NS, refreshNs, 3600);

  for (size_t index = 100; index < run.phases.size(); ++index) {
    const auto fromPresent = std::min(run.phases[index], 1.0 - run.phases[index]);
    EXPECT_LE(fromPresent, 0.05) << "at frame " << index;
  }

  EXPECT_GE(run.doubles, 7);
  EXPECT_LE(run.doubles, 10);
  EXPECT_EQ(run.repeats, 0);
}

// TODO
// What the content is owed is owed whatever the clock is stepping by, so only a game being loaded or
// unpaused starts the tally over
TEST(EmulationRateControllerTest, WhatIsOwedSurvivesLosingTheGrid) {
  auto controller = heldNativeOf(SNES_FPS, 59.959);
  const auto refreshNs = static_cast<int64_t>(SECOND_NS / 59.959);

  runHeld(controller, SECOND_NS, refreshNs, 300);
  const auto owedNs = controller.getSlipDebtNs();
  EXPECT_GT(owedNs, 0);

  // A gap in the presents starts the grid over, so the clock leaves the hold for its own period
  controller.notePresent(controller.getNextDeadlineNs() + SECOND_NS);
  controller.framesDue(controller.getNextDeadlineNs());
  EXPECT_FALSE(controller.getPhase().isLocked());
  EXPECT_EQ(controller.getSlipDebtNs(), owedNs);

  controller.reset();
  EXPECT_EQ(controller.getSlipDebtNs(), 0);
}

TEST(EmulationRateControllerTest, TheMarginFollowsTheWakePeak) {
  auto controller = displayOf(60.0, 60.0);
  controller.noteFrameDuration(4000000);
  controller.notePassDuration(2000000);
  const auto restingTarget = controller.getPhaseTarget();

  EXPECT_EQ(controller.getWakeMarginNs(), EmulationRateController::WAKE_MARGIN_NS);

  controller.noteWakeLateness(6000000);
  EXPECT_EQ(controller.getWakeMarginNs(), 6000000);
  EXPECT_NEAR(controller.getPhaseTarget(), restingTarget - 2000000.0 / (1e9 / 60.0), 0.001);

  controller.noteWakeLateness(50000000);
  EXPECT_EQ(controller.getWakeMarginNs(), EmulationRateController::WAKE_PEAK_CAP_NS);

  for (auto frame = 0; frame < 600; ++frame) {
    controller.noteWakeLateness(0);
  }

  EXPECT_EQ(controller.getWakeMarginNs(), EmulationRateController::WAKE_MARGIN_NS);
  EXPECT_NEAR(controller.getPhaseTarget(), restingTarget, 0.001);
}

// TODO
// The asserted margin is a quarter of a 60 Hz refresh and most of a 240 Hz one, so it is capped by
// the refresh it has to fit inside. What a late wake really costs is never capped
TEST(EmulationRateControllerTest, TheAssertedMarginFitsInsideTheRefresh) {
  EXPECT_EQ(displayOf(60.0, 60.0).getWakeMarginNs(), EmulationRateController::WAKE_MARGIN_NS);

  auto fast = displayOf(60.0, 240.0);
  const auto refreshNs = static_cast<int64_t>(SECOND_NS / 240.0);

  EXPECT_EQ(fast.getWakeMarginNs(), refreshNs / EmulationRateController::MARGIN_SHARE_OF_REFRESH);

  fast.noteWakeLateness(6000000);
  EXPECT_EQ(fast.getWakeMarginNs(), 6000000);
}

TEST(EmulationRateControllerTest, APinnedPhaseTargetWins) {
  EmulationRateController controller;
  controller.configure({.mode = SyncMode::Display, .contentFps = 60.0, .displayHz = 60.0, .presentationLocked = true});

  controller.setPhaseTarget(0.3);
  controller.noteFrameDuration(4000000);
  EXPECT_DOUBLE_EQ(controller.getPhaseTarget(), 0.3);

  controller.setPhaseTarget(-1.0);
  EXPECT_GT(controller.getPhaseTarget(), 0.3);
}

} // namespace firelight::emulation

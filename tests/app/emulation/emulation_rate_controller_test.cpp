#include <emulation/emulation_rate_controller.hpp>
#include <gtest/gtest.h>

namespace firelight::emulation {

namespace {
constexpr int64_t SECOND_NS = 1000000000LL;
constexpr double SNES_FPS = 60.0988;

EmulationRateController fixedAt(const double fps) {
  EmulationRateController controller;
  controller.configure({.mode = SyncMode::Fixed, .contentFps = fps});
  return controller;
}

// Display counts presents as refreshes, which only stands up when presentation waits for the
// display — so that is the case these are about unless a test says otherwise
EmulationRateController displayOf(const double contentFps, const double displayHz,
                                  const bool presentationLocked = true) {
  EmulationRateController controller;
  controller.configure({.mode = SyncMode::Display,
                        .contentFps = contentFps,
                        .displayHz = displayHz,
                        .presentationLocked = presentationLocked});
  return controller;
}

EmulationRateController autoOf(const double contentFps, const double displayHz, const bool audioAvailable,
                               const bool presentationLocked) {
  EmulationRateController controller;
  controller.configure({.mode = SyncMode::Auto,
                        .contentFps = contentFps,
                        .displayHz = displayHz,
                        .audioAvailable = audioAvailable,
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
} // namespace

TEST(EmulationRateControllerTest, FixedRunsOneFramePerInterval) {
  auto controller = fixedAt(60.0);
  const auto intervalNs = SECOND_NS / 60;

  auto now = SECOND_NS;
  EXPECT_EQ(controller.framesDue(now), 1);

  for (auto i = 0; i < 10; ++i) {
    now += intervalNs;
    EXPECT_EQ(controller.framesDue(now), 1) << "at step " << i;
  }
}

TEST(EmulationRateControllerTest, FixedRunsNothingBeforeTheFrameIsDue) {
  auto controller = fixedAt(60.0);

  auto now = SECOND_NS;
  controller.framesDue(now);

  // A third of the way into the frame, nothing is owed yet
  now += SECOND_NS / 180;
  EXPECT_EQ(controller.framesDue(now), 0);
}

// The rate the core reports is the truth, and a rate that doesn't divide the second evenly is the
// case where an accumulator earns its keep
TEST(EmulationRateControllerTest, FixedHoldsAnAwkwardRateOverAMinute) {
  auto controller = fixedAt(SNES_FPS);

  const auto frames = runFor(controller, 60.0, SECOND_NS / 1000);

  EXPECT_NEAR(frames, 60.0 * SNES_FPS, 2.0);
}

TEST(EmulationRateControllerTest, FixedHoldsANonStandardRate) {
  auto controller = fixedAt(48.0);

  const auto frames = runFor(controller, 30.0, SECOND_NS / 1000);

  EXPECT_NEAR(frames, 30.0 * 48.0, 2.0);
}

// The case that started all of this: 60 fps content on a 120 Hz panel has to be every other
// refresh, exactly, rather than whatever the audio buffer happened to allow
TEST(EmulationRateControllerTest, DisplayRunsEveryOtherRefreshAt120For60) {
  auto controller = displayOf(60.0, 120.0);

  EXPECT_EQ(controller.getRefreshesPerFrame(), 2);

  for (auto i = 0; i < 10; ++i) {
    EXPECT_EQ(controller.framesDueOnPresent(), 0) << "refresh " << i * 2;
    EXPECT_EQ(controller.framesDueOnPresent(), 1) << "refresh " << i * 2 + 1;
  }
}

TEST(EmulationRateControllerTest, DisplayRunsEveryRefreshWhenTheRatesMatch) {
  auto controller = displayOf(60.0, 60.0);

  EXPECT_EQ(controller.getRefreshesPerFrame(), 1);

  for (auto i = 0; i < 5; ++i) {
    EXPECT_EQ(controller.framesDueOnPresent(), 1);
  }
}

// A 60 Hz panel can't show 60.0988, so the game runs at the panel's rate and the audio is stretched
// to match — the same bargain the old monitor mode struck
TEST(EmulationRateControllerTest, DisplayBendsAudioWhenItRoundsTheContentRate) {
  auto controller = displayOf(SNES_FPS, 60.0);

  EXPECT_EQ(controller.getRefreshesPerFrame(), 1);
  EXPECT_NEAR(controller.getEffectiveFps(), 60.0, 0.001);
  EXPECT_NEAR(controller.getAudioRatio(), 60.0 / SNES_FPS, 0.0001);
}

// 120/48 is 2.5 — there is no whole number of refreshes that shows 48 fps, so the mode declines and
// leaves the clock alone rather than running the game at the wrong speed
TEST(EmulationRateControllerTest, DisplayDeclinesARateTheRefreshCannotRepresent) {
  auto controller = displayOf(48.0, 120.0);

  EXPECT_EQ(controller.getRefreshesPerFrame(), 0);
  EXPECT_NEAR(controller.getEffectiveFps(), 48.0, 0.001);
  EXPECT_NEAR(controller.getAudioRatio(), 1.0, 0.0001);
}

TEST(EmulationRateControllerTest, DisplayFallsBackToTheClockWhenItDeclines) {
  auto controller = displayOf(48.0, 120.0);

  const auto frames = runFor(controller, 10.0, SECOND_NS / 1000);

  EXPECT_NEAR(frames, 10.0 * 48.0, 2.0);
}

TEST(EmulationRateControllerTest, DisplayIgnoresAnUnknownRefreshRate) {
  auto controller = displayOf(60.0, 0.0);

  EXPECT_EQ(controller.getRefreshesPerFrame(), 0);
  EXPECT_NEAR(controller.getEffectiveFps(), 60.0, 0.001);
}

// Audio has no rate of its own: a frame is due when there is room for the audio it makes, and not
// otherwise. The device's consumption is the clock
TEST(EmulationRateControllerTest, AudioRunsAFrameWhenTheSinkHasRoom) {
  EmulationRateController controller;
  controller.configure({.mode = SyncMode::Audio, .contentFps = 60.0});

  controller.setAudioBufferLevel(0.1F);

  EXPECT_EQ(controller.framesDue(SECOND_NS), 1);
}

TEST(EmulationRateControllerTest, AudioRunsNothingWhenTheSinkIsFull) {
  EmulationRateController controller;
  controller.configure({.mode = SyncMode::Audio, .contentFps = 60.0});

  controller.setAudioBufferLevel(0.9F);

  EXPECT_EQ(controller.framesDue(SECOND_NS), 0);
}

// Asked over and over with the buffer draining, it keeps answering yes — which is what lets a device
// running slightly fast or slow set the pace instead of a clock that can only guess at it
TEST(EmulationRateControllerTest, AudioKeepsAnsweringWhileTheSinkIsDraining) {
  EmulationRateController controller;
  controller.configure({.mode = SyncMode::Audio, .contentFps = 60.0});
  controller.setAudioBufferLevel(0.2F);

  auto frames = 0;
  for (auto i = 0; i < 10; ++i) {
    frames += controller.framesDue(SECOND_NS + i);
  }

  EXPECT_EQ(frames, 10);
}

// The clock means nothing here: no time passing makes a frame due if the sink is already full
TEST(EmulationRateControllerTest, AudioIgnoresTheClock) {
  EmulationRateController controller;
  controller.configure({.mode = SyncMode::Audio, .contentFps = 60.0});
  controller.setAudioBufferLevel(0.8F);

  EXPECT_EQ(controller.framesDue(SECOND_NS), 0);
  EXPECT_EQ(controller.framesDue(SECOND_NS + 10 * SECOND_NS), 0);
  EXPECT_EQ(controller.getNextDeadlineNs(), 0);
}

// TODO
// A sink that cannot be read is one being rebuilt or one that was lost, and a frame is the only
// thing that calls into it — so stopping on an unreadable level is a game that never starts again.
// The clock takes over until there is a level to read
TEST(EmulationRateControllerTest, AudioFallsBackToTheClockWhenTheSinkCannotBeRead) {
  EmulationRateController controller;
  controller.configure({.mode = SyncMode::Audio, .contentFps = 60.0});
  controller.setAudioBufferLevel(-1.0F);

  EXPECT_EQ(controller.framesDue(SECOND_NS), 1) << "the first frame runs immediately";
  EXPECT_EQ(controller.framesDue(SECOND_NS + SECOND_NS / 60), 1) << "and then at the content rate";

  controller.setAudioBufferLevel(0.8F);
  EXPECT_EQ(controller.framesDue(SECOND_NS + 2 * SECOND_NS / 60), 0) << "a readable full sink gates again";
}

// Time the player didn't experience isn't owed. A breakpoint or a suspended laptop must not come
// back as a burst of fast-forward
TEST(EmulationRateControllerTest, ALongStallDoesNotReplayAsFastForward) {
  auto controller = fixedAt(60.0);

  auto now = SECOND_NS;
  controller.framesDue(now);

  now += 10 * SECOND_NS;
  EXPECT_EQ(controller.framesDue(now), 1);
}

TEST(EmulationRateControllerTest, CatchUpIsCapped) {
  auto controller = fixedAt(60.0);

  auto now = SECOND_NS;
  controller.framesDue(now);

  // Late by ten frames, but still inside what counts as continuous play
  now += SECOND_NS / 6;
  EXPECT_EQ(controller.framesDue(now), EmulationRateController::MAX_CATCH_UP_FRAMES);
}

TEST(EmulationRateControllerTest, DroppedDebtDoesNotAccumulate) {
  auto controller = fixedAt(60.0);
  const auto intervalNs = SECOND_NS / 60;

  auto now = SECOND_NS;
  controller.framesDue(now);

  now += SECOND_NS / 6;
  controller.framesDue(now);

  // Back on cadence, the very next interval owes one frame and no arrears
  now += intervalNs;
  EXPECT_EQ(controller.framesDue(now), 1);
}

TEST(EmulationRateControllerTest, ResetDropsThePhase) {
  auto controller = fixedAt(60.0);

  auto now = SECOND_NS;
  controller.framesDue(now);
  now += SECOND_NS / 90;
  controller.framesDue(now);

  controller.reset();

  // The first call after a reset starts a fresh cadence rather than owing the gap
  now += 10 * SECOND_NS;
  EXPECT_EQ(controller.framesDue(now), 1);
}

TEST(EmulationRateControllerTest, ChangingModeStartsAFreshCadence) {
  EmulationRateController controller;
  controller.configure({.mode = SyncMode::Fixed, .contentFps = 60.0});

  auto now = SECOND_NS;
  controller.framesDue(now);

  controller.configure({.mode = SyncMode::Display, .contentFps = 60.0, .displayHz = 120.0, .presentationLocked = true});

  EXPECT_EQ(controller.getRefreshesPerFrame(), 2);
  EXPECT_EQ(controller.framesDueOnPresent(), 0);
  EXPECT_EQ(controller.framesDueOnPresent(), 1);
}

TEST(EmulationRateControllerTest, ChangingTheRefreshRateRelocks) {
  auto controller = displayOf(60.0, 60.0);
  ASSERT_EQ(controller.getRefreshesPerFrame(), 1);

  controller.configure({.mode = SyncMode::Display, .contentFps = 60.0, .displayHz = 120.0, .presentationLocked = true});

  EXPECT_EQ(controller.getRefreshesPerFrame(), 2);
}

// Display counts refreshes, so a clock tick owes it nothing
TEST(EmulationRateControllerTest, DisplayOwesNothingToTheClock) {
  auto controller = displayOf(60.0, 120.0);

  EXPECT_EQ(controller.getNextDeadlineNs(), 0);
  EXPECT_EQ(runFor(controller, 5.0, SECOND_NS / 1000), 0);
}

// A refresh the display can't divide into the content rate leaves Display on the clock instead
TEST(EmulationRateControllerTest, DisplayRunsOnTheClockWhenItDeclines) {
  auto controller = displayOf(48.0, 120.0);

  // A deadline only exists once a cadence has started
  EXPECT_EQ(controller.framesDue(SECOND_NS), 1);
  EXPECT_NE(controller.getNextDeadlineNs(), 0);
  EXPECT_NEAR(runFor(controller, 10.0, SECOND_NS / 1000), 10.0 * 48.0, 2.0);
}

TEST(EmulationRateControllerTest, TheDeadlineIsOneIntervalAfterTheLastFrame) {
  auto controller = fixedAt(60.0);

  const auto now = SECOND_NS;
  controller.framesDue(now);

  EXPECT_NEAR(static_cast<double>(controller.getNextDeadlineNs() - now), static_cast<double>(SECOND_NS) / 60.0,
              1000000.0);
}

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
    auto controller = autoOf(c.contentFps, c.displayHz, true, c.presentationLocked);

    EXPECT_NE(controller.getResolvedMode(), SyncMode::Audio)
        << c.contentFps << "fps on " << c.displayHz << "Hz, locked=" << c.presentationLocked;
  }
}

TEST(EmulationRateControllerTest, AutoIsUnmovedByWhetherThereIsASink) {
  auto withSink = autoOf(SNES_FPS, 60.0, true, false);
  auto without = autoOf(SNES_FPS, 60.0, false, false);

  EXPECT_EQ(withSink.getResolvedMode(), without.getResolvedMode());
  EXPECT_NEAR(withSink.getEffectiveFps(), without.getEffectiveFps(), 0.001);
}

// TODO
// The one case where counting refreshes beats a clock: with presentation waiting for the display a
// present IS a refresh, so a frame lands on an exact number of them with no beat left to drift
TEST(EmulationRateControllerTest, AutoCountsRefreshesWhenPresentationWaitsForThem) {
  auto controller = autoOf(SNES_FPS, 60.0, true, true);

  EXPECT_EQ(controller.getResolvedMode(), SyncMode::Display);
  EXPECT_EQ(controller.getRefreshesPerFrame(), 1);
  EXPECT_NEAR(controller.getEffectiveFps(), 60.0, 0.001);
  EXPECT_NEAR(controller.getAudioRatio(), 60.0 / SNES_FPS, 0.0001);
}

TEST(EmulationRateControllerTest, AutoHoldsAFrameForTwoRefreshesOnAHighRefreshPanel) {
  auto controller = autoOf(60.0, 120.0, true, true);

  EXPECT_EQ(controller.getResolvedMode(), SyncMode::Display);
  EXPECT_EQ(controller.getRefreshesPerFrame(), 2);

  for (auto i = 0; i < 5; ++i) {
    EXPECT_EQ(controller.framesDueOnPresent(), 0) << "refresh " << i * 2;
    EXPECT_EQ(controller.framesDueOnPresent(), 1) << "refresh " << i * 2 + 1;
  }
}

// TODO
// Unlocked, a present says nothing about when a refresh happened, so the rate is kept but taken from
// a clock instead of from the presents
TEST(EmulationRateControllerTest, AutoRunsAClockAtTheDisplayRateWhenPresentationDoesNotWait) {
  auto controller = autoOf(SNES_FPS, 60.0, true, false);

  EXPECT_EQ(controller.getResolvedMode(), SyncMode::Fixed);
  EXPECT_EQ(controller.getRefreshesPerFrame(), 0);
  EXPECT_NEAR(controller.getEffectiveFps(), 60.0, 0.001);
  EXPECT_NEAR(controller.getAudioRatio(), 60.0 / SNES_FPS, 0.0001);
  EXPECT_NEAR(runFor(controller, 10.0, SECOND_NS / 1000), 10.0 * 60.0, 2.0);
}

// 120/48 is 2.5, so no whole number of refreshes shows 48 fps and the display has nothing to offer
TEST(EmulationRateControllerTest, AutoFallsBackToTheContentRateWhenNothingFits) {
  auto controller = autoOf(48.0, 120.0, true, true);

  EXPECT_EQ(controller.getResolvedMode(), SyncMode::Fixed);
  EXPECT_EQ(controller.getRefreshesPerFrame(), 0);
  EXPECT_NEAR(controller.getEffectiveFps(), 48.0, 0.001);
  EXPECT_NEAR(controller.getAudioRatio(), 1.0, 0.0001);
  EXPECT_NEAR(runFor(controller, 10.0, SECOND_NS / 1000), 10.0 * 48.0, 2.0);
}

TEST(EmulationRateControllerTest, AutoIgnoresAnUnknownRefreshRate) {
  auto controller = autoOf(60.0, 0.0, true, true);

  EXPECT_EQ(controller.getResolvedMode(), SyncMode::Fixed);
  EXPECT_NEAR(controller.getEffectiveFps(), 60.0, 0.001);
}

// Turning vsync on or off changes which is right, so it has to change which is used
TEST(EmulationRateControllerTest, AutoSwitchesModeWhenPresentationLockChanges) {
  auto controller = autoOf(60.0, 60.0, true, false);
  ASSERT_EQ(controller.getResolvedMode(), SyncMode::Fixed);

  controller.framesDue(SECOND_NS);
  ASSERT_NE(controller.getNextDeadlineNs(), 0);

  controller.configure({.mode = SyncMode::Auto, .contentFps = 60.0, .displayHz = 60.0, .presentationLocked = true});

  EXPECT_EQ(controller.getResolvedMode(), SyncMode::Display);
  EXPECT_EQ(controller.getNextDeadlineNs(), 0);
}

// TODO
// Taking the display's rate means stretching the audio by the difference, so what the tolerance has
// to bound is how much of that can be heard rather than how near the two numbers look
TEST(EmulationRateControllerTest, AutoDeclinesADisplayRateFarEnoughOutToBeHeard) {
  // 3.4% out, which is most of a semitone of stretch on everything the game plays
  auto controller = autoOf(58.0, 60.0, true, false);

  EXPECT_EQ(controller.getResolvedMode(), SyncMode::Fixed);
  EXPECT_NEAR(controller.getEffectiveFps(), 58.0, 0.001);
  EXPECT_NEAR(controller.getAudioRatio(), 1.0, 0.0001);
}

TEST(EmulationRateControllerTest, AutoTakesTheRatesRealContentReports) {
  // NTSC, the SNES, and the Game Boy Advance, all against a 60 Hz panel
  for (const auto contentFps : {59.94, 60.0988, 59.7275}) {
    auto controller = autoOf(contentFps, 60.0, true, false);

    EXPECT_EQ(controller.getResolvedMode(), SyncMode::Fixed) << "at " << contentFps;
    EXPECT_NEAR(controller.getEffectiveFps(), 60.0, 0.001) << "at " << contentFps;
  }
}

// A mode named outright is used as named, however well or badly it fits
TEST(EmulationRateControllerTest, AnExplicitModeIsNotSecondGuessed) {
  auto controller = fixedAt(48.0);

  EXPECT_EQ(controller.getResolvedMode(), SyncMode::Fixed);
  EXPECT_NEAR(controller.getEffectiveFps(), 48.0, 0.001);
}

// Counting presents that don't wait for the display would run the game at whatever rate they happen
// to arrive at — which is exactly how monitor mode came to run at twice speed with vsync off
TEST(EmulationRateControllerTest, DisplayRunsOnAClockWhenPresentationDoesNotWaitForTheDisplay) {
  auto controller = displayOf(SNES_FPS, 60.0, false);

  EXPECT_EQ(controller.getResolvedMode(), SyncMode::Fixed);
  EXPECT_EQ(controller.getRefreshesPerFrame(), 0);
  EXPECT_NEAR(controller.getEffectiveFps(), 60.0, 0.001);
  EXPECT_NEAR(controller.getAudioRatio(), 60.0 / SNES_FPS, 0.0001);
}

TEST(EmulationRateControllerTest, DisplayIgnoresPresentsWhenPresentationDoesNotWaitForTheDisplay) {
  auto controller = displayOf(60.0, 120.0, false);

  for (auto i = 0; i < 10; ++i) {
    EXPECT_EQ(controller.framesDueOnPresent(), 0) << "present " << i;
  }

  EXPECT_NEAR(runFor(controller, 10.0, SECOND_NS / 1000), 10.0 * 60.0, 2.0);
}

// The predicate that decides whether a reconfigure keeps the phase or starts over. Both directions
// are pinned, because either mistake is silent: dropping a phase that was fine costs a frame, and
// keeping one that isn't leaves the cadence describing a rate the game no longer runs at
TEST(EmulationRateControllerTest, ChangingTheRateStartsAFreshCadence) {
  auto controller = fixedAt(60.0);

  controller.framesDue(SECOND_NS);
  ASSERT_NE(controller.getNextDeadlineNs(), 0);

  controller.configure({.mode = SyncMode::Fixed, .contentFps = 30.0});

  EXPECT_EQ(controller.getNextDeadlineNs(), 0);
}

TEST(EmulationRateControllerTest, ReconfiguringWithTheSameCadenceKeepsThePhase) {
  auto controller = fixedAt(60.0);

  controller.framesDue(SECOND_NS);
  const auto deadlineNs = controller.getNextDeadlineNs();
  ASSERT_NE(deadlineNs, 0);

  controller.configure({.mode = SyncMode::Fixed, .contentFps = 60.0});

  EXPECT_EQ(controller.getNextDeadlineNs(), deadlineNs);
}

TEST(EmulationRateControllerTest, ChangingHowManyRefreshesAFrameHoldsStartsAFreshCount) {
  auto controller = displayOf(60.0, 120.0);
  ASSERT_EQ(controller.getRefreshesPerFrame(), 2);

  // Part-way through one frame's two refreshes
  ASSERT_EQ(controller.framesDueOnPresent(), 0);

  controller.configure({.mode = SyncMode::Display, .contentFps = 60.0, .displayHz = 240.0, .presentationLocked = true});
  ASSERT_EQ(controller.getRefreshesPerFrame(), 4);

  // Four refreshes from here, not three: the one already counted belonged to a cadence that is gone
  EXPECT_EQ(controller.framesDueOnPresent(), 0);
  EXPECT_EQ(controller.framesDueOnPresent(), 0);
  EXPECT_EQ(controller.framesDueOnPresent(), 0);
  EXPECT_EQ(controller.framesDueOnPresent(), 1);
}

} // namespace firelight::emulation

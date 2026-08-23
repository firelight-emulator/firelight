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

EmulationRateController displayOf(const double contentFps, const double displayHz) {
  EmulationRateController controller;
  controller.configure({.mode = SyncMode::Display, .contentFps = contentFps, .displayHz = displayHz});
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

TEST(EmulationRateControllerTest, AudioRunsNothingWhenTheSinkCannotBeRead) {
  EmulationRateController controller;
  controller.configure({.mode = SyncMode::Audio, .contentFps = 60.0});
  controller.setAudioBufferLevel(-1.0F);

  EXPECT_EQ(controller.framesDue(SECOND_NS), 0);
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

  controller.configure({.mode = SyncMode::Display, .contentFps = 60.0, .displayHz = 120.0});

  EXPECT_EQ(controller.getRefreshesPerFrame(), 2);
  EXPECT_EQ(controller.framesDueOnPresent(), 0);
  EXPECT_EQ(controller.framesDueOnPresent(), 1);
}

TEST(EmulationRateControllerTest, ChangingTheRefreshRateRelocks) {
  auto controller = displayOf(60.0, 60.0);
  ASSERT_EQ(controller.getRefreshesPerFrame(), 1);

  controller.configure({.mode = SyncMode::Display, .contentFps = 60.0, .displayHz = 120.0});

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

} // namespace firelight::emulation

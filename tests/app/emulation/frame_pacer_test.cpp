// TODO: NEEDS REVIEW
#include <emulation/frame_pacer.hpp>
#include <gtest/gtest.h>

using firelight::emulation::FramePacer;
using firelight::emulation::PacingContext;
using firelight::emulation::SyncMode;

namespace {
constexpr int64_t SECOND_NS = 1000000000;
constexpr int64_t FRAME_NS = SECOND_NS / 60;
constexpr double DISPLAY_120 = 120.0;

/**
 * Puts a pacer past the states that hold frames back, so a test can start from a running game
 */
void startRunning(FramePacer &pacer, const SyncMode mode, const double contentFps, const double displayHz,
                  const bool presentationLocked) {
  pacer.configure(
      {.mode = mode, .contentFps = contentFps, .displayHz = displayHz, .presentationLocked = presentationLocked});
  pacer.setReady(true);
}

/**
 * Feeds the pacer frames reaching the display at exactly the refresh rate
 */
void submitRefreshes(FramePacer &pacer, const int count, int64_t &nowNs, const int64_t periodNs) {
  for (auto i = 0; i < count; ++i) {
    nowNs += periodNs;
    pacer.noteSubmit(nowNs);
  }
}
} // namespace

// TODO
// Nothing has come up yet, and a frame run before there is anything to run it is a frame lost
TEST(FramePacerTest, RunsNothingUntilThereIsAnEmulator) {
  FramePacer pacer;
  pacer.configure({.mode = SyncMode::Fixed, .contentFps = 60.0});

  pacer.noteSubmit(SECOND_NS);
  EXPECT_EQ(pacer.tick(SECOND_NS).framesToRun, 0);

  pacer.setReady(true);
  pacer.noteSubmit(SECOND_NS + FRAME_NS);
  EXPECT_GT(pacer.tick(SECOND_NS + FRAME_NS).framesToRun, 0);
}

// TODO
// A game in the quick menu owes nothing for the time it spent there
TEST(FramePacerTest, APauseOwesNothingForTheTimeItLasted) {
  FramePacer pacer;
  startRunning(pacer, SyncMode::Fixed, 60.0, 0.0, false);

  auto nowNs = SECOND_NS;
  pacer.tick(nowNs);

  pacer.setPaused(true);
  pacer.tick(nowNs + 10 * SECOND_NS);

  pacer.setPaused(false);
  EXPECT_LE(pacer.tick(nowNs + 10 * SECOND_NS + FRAME_NS).framesToRun, 1)
      << "ten seconds paused came back as a burst of frames";
}

// TODO
// The first frame after a stop has nothing to measure against, and charging it the length of the
// stop would count refreshes that happened while nobody was watching
TEST(FramePacerTest, TheFirstSubmissionHasNoGap) {
  FramePacer pacer;
  startRunning(pacer, SyncMode::Display, 60.0, DISPLAY_120, true);

  EXPECT_EQ(pacer.noteSubmit(SECOND_NS), 0);
  EXPECT_EQ(pacer.noteSubmit(SECOND_NS + FRAME_NS), FRAME_NS);
}

// TODO
// Display holds a frame for a whole number of the frames that reach the display, so two refreshes on
// a 120Hz panel is one frame of 60Hz content
TEST(FramePacerTest, DisplayRunsAFrameEveryOtherRefreshAt120) {
  FramePacer pacer;
  startRunning(pacer, SyncMode::Display, 60.0, DISPLAY_120, true);
  const auto periodNs = static_cast<int64_t>(1e9 / DISPLAY_120);

  auto nowNs = SECOND_NS;
  pacer.noteSubmit(nowNs);
  pacer.tick(nowNs);

  auto frames = 0;
  for (auto i = 0; i < 60; ++i) {
    submitRefreshes(pacer, 2, nowNs, periodNs);
    frames += pacer.tick(nowNs).framesToRun;
  }

  EXPECT_EQ(frames, 60) << "sixty frames should take a hundred and twenty refreshes";
}

// TODO
// Nothing reaching the display means nothing is drawing, and frames run in the pass that shows them
TEST(FramePacerTest, AsksForADrawWhenNothingHasReachedTheDisplay) {
  FramePacer pacer;
  startRunning(pacer, SyncMode::Display, 60.0, DISPLAY_120, true);

  auto nowNs = SECOND_NS;
  pacer.noteSubmit(nowNs);
  pacer.tick(nowNs);

  const auto decision = pacer.tick(nowNs + FramePacer::RENDER_STALL_NS + 1);

  EXPECT_TRUE(decision.shouldRequestRender);
  EXPECT_EQ(decision.framesToRun, 0) << "a frame with nobody drawing is a frame that never appears";
}

// TODO
// Time spent not drawing is not time the player experienced, so it must not come back afterwards as
// a run of frames nobody is waiting for
TEST(FramePacerTest, AStallDoesNotBankFrames) {
  FramePacer pacer;
  startRunning(pacer, SyncMode::Fixed, 60.0, 0.0, false);
  const auto periodNs = static_cast<int64_t>(1e9 / DISPLAY_120);

  auto nowNs = SECOND_NS;
  pacer.noteSubmit(nowNs);
  pacer.tick(nowNs);

  nowNs += FramePacer::RENDER_STALL_NS + 1;
  pacer.tick(nowNs);

  submitRefreshes(pacer, 1, nowNs, periodNs);
  EXPECT_LE(pacer.tick(nowNs).framesToRun, 1) << "a quarter second of not drawing came back as a burst";
}

// TODO
// Audio gates on there being room rather than on a clock, so a full sink runs nothing however long
// it has been
TEST(FramePacerTest, AudioRunsNothingWhileTheSinkIsFull) {
  FramePacer pacer;
  startRunning(pacer, SyncMode::Audio, 60.0, 0.0, false);

  auto nowNs = SECOND_NS;
  pacer.noteSubmit(nowNs);
  pacer.tick(nowNs);

  nowNs += FRAME_NS;
  pacer.setAudioBufferLevel(0.9F);
  pacer.noteSubmit(nowNs);
  EXPECT_EQ(pacer.tick(nowNs).framesToRun, 0);

  nowNs += FRAME_NS;
  pacer.setAudioBufferLevel(0.1F);
  pacer.noteSubmit(nowNs);
  EXPECT_GT(pacer.tick(nowNs).framesToRun, 0);
}

// TODO
// Reconfiguring mid-game must not leave the phase of the old cadence describing the new one
TEST(FramePacerTest, ReconfiguringForgetsTheOldCadence) {
  FramePacer pacer;
  startRunning(pacer, SyncMode::Display, 60.0, DISPLAY_120, true);
  const auto periodNs = static_cast<int64_t>(1e9 / DISPLAY_120);

  auto nowNs = SECOND_NS;
  pacer.noteSubmit(nowNs);
  submitRefreshes(pacer, 1, nowNs, periodNs);

  pacer.configure({.mode = SyncMode::Fixed, .contentFps = 60.0, .displayHz = DISPLAY_120});

  EXPECT_EQ(pacer.getResolvedMode(), SyncMode::Fixed);
  EXPECT_EQ(pacer.getRefreshesPerFrame(), 0) << "a refresh count means nothing to a clock";
}

// TODO
// A clock still owes frames when nothing is reaching the display, because it does not count them —
// but the stall rule holds them back all the same
TEST(FramePacerTest, ResetForgetsWhatWasOwed) {
  FramePacer pacer;
  startRunning(pacer, SyncMode::Fixed, 60.0, 0.0, false);

  auto nowNs = SECOND_NS;
  pacer.noteSubmit(nowNs);
  pacer.tick(nowNs);

  pacer.reset();
  EXPECT_EQ(pacer.tick(nowNs + 10 * SECOND_NS).framesToRun, 0) << "a reset pacer starts owing nothing";
}

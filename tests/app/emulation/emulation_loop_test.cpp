// TODO: NEEDS REVIEW
#include "emulation_service_fixture.hpp"

#include <emulation/emulation_loop.hpp>
#include <emulation/emulator_controller.hpp>
#include <emulation/emulator_instance.hpp>
#include <gtest/gtest.h>

namespace firelight::emulation {

namespace {
constexpr int64_t SECOND_NS = 1000000000;
constexpr int64_t REFRESH_NS = 16666667;

/** Time that only moves when the loop waits */
class FakeClock : public ILoopClock {
public:
  int64_t nowNs() override { return m_nowNs; }

  void waitUntil(const int64_t untilNs) override { m_nowNs = std::max(m_nowNs, untilNs); }

  int64_t m_nowNs = SECOND_NS;
};

/** Takes the core's frames and drops them */
class FakeReceiver : public libretro::IVideoDataReceiver {
public:
  void receive(const void *, unsigned, unsigned, size_t) override { m_frames++; }

  retro_hw_context_type getPreferredHwRender() override { return RETRO_HW_CONTEXT_NONE; }

  void setHwRenderInterface(retro_hw_render_callback *) override {}

  void setSystemAVInfo(retro_system_av_info *) override {}

  void setPixelFormat(retro_pixel_format *) override {}

  void setScreenRotation(unsigned) override {}

  void setHwRenderContextNegotiationInterface(retro_hw_render_context_negotiation_interface *) override {}

  void getHwRenderInterface(retro_hw_render_interface **) override {}

  int m_frames = 0;
};

class FakeController : public IEmulatorController {
public:
  float playbackMultiplier() const override { return m_multiplier; }

  void setPlaybackMultiplier(const float multiplier) override { m_multiplier = multiplier; }

  bool paused() const override { return m_paused; }

  void setPaused(const bool paused) override { m_paused = paused; }

  void advanceOneFrame() override {}

  void writeSuspendPoint(int) override {}

  void loadSuspendPoint(int) override {}

  void captureScreenshot() override {}

  void captureVideoClip() override {}

  float m_multiplier = 1.0F;
  bool m_paused = false;
};

/** A loop at a fixed 60 fps over the fixture's service, with a display presenting every refresh */
class EmulationLoopTest : public EmulationServiceFixture {
protected:
  FakeClock m_clock;
  FakeController m_controller;
  FakeReceiver m_receiver;
  bool m_displayReady = true;
  bool m_framesAllowed = true;
  int m_passes = 0;
  std::unique_ptr<EmulationLoop> m_loop;

  void SetUp() override {
    EmulationServiceFixture::SetUp();
    m_loop = std::make_unique<EmulationLoop>(m_clock, *m_emulationService, m_controller,
                                             LoopHooks{.receiver = [this]() -> libretro::IVideoDataReceiver * {
                                                         return m_displayReady ? &m_receiver : nullptr;
                                                       },
                                                       .prepareForFrames = [this] { return m_framesAllowed; },
                                                       .requestPass = [this] { m_passes++; }});
    m_loop->getPacer().configure(
        {.mode = SyncMode::Fixed, .contentFps = 60.0, .displayHz = 60.0, .presentationLocked = true});
  }

  void TearDown() override {
    m_loop.reset();
    EmulationServiceFixture::TearDown();
  }

  /**
   * Iterates for `seconds` of simulated time, feeding a present every refresh
   */
  void runFor(const double seconds, const bool presenting = true) {
    const auto endNs = m_clock.m_nowNs + static_cast<int64_t>(seconds * SECOND_NS);
    auto nextPresentNs = m_clock.m_nowNs;

    while (m_clock.m_nowNs < endNs) {
      while (presenting && nextPresentNs <= m_clock.m_nowNs) {
        m_loop->getPacer().noteSubmit(nextPresentNs);
        nextPresentNs += REFRESH_NS;
      }

      m_loop->runOnce();
    }
  }
};
} // namespace

TEST_F(EmulationLoopTest, BringsTheGameUpAndRunsAFrameEveryPeriod) {
  auto *instance = loadGame();
  ASSERT_NE(instance, nullptr);

  runFor(1.0);

  EXPECT_TRUE(instance->isInitialized());
  EXPECT_NEAR(m_fakeCore->frameCount(), 60, 2);
  EXPECT_EQ(m_passes, m_fakeCore->frameCount());
}

TEST_F(EmulationLoopTest, WaitsForTheDisplayBeforeBringingTheGameUp) {
  auto *instance = loadGame();
  ASSERT_NE(instance, nullptr);
  m_displayReady = false;

  runFor(0.5);

  EXPECT_FALSE(instance->isInitialized());
  EXPECT_EQ(m_passes, 0);

  m_displayReady = true;
  runFor(0.5);

  EXPECT_TRUE(instance->isInitialized());
  EXPECT_GT(m_fakeCore->frameCount(), 20);
}

TEST_F(EmulationLoopTest, RunsNothingUntilFramesAreAllowed) {
  auto *instance = loadGame();
  ASSERT_NE(instance, nullptr);
  m_framesAllowed = false;

  runFor(0.5);

  EXPECT_TRUE(instance->isInitialized());
  EXPECT_EQ(m_fakeCore->frameCount(), 0);
}

TEST_F(EmulationLoopTest, PausedRunsNothingButQueuedWorkStillHappens) {
  auto *instance = loadGame();
  ASSERT_NE(instance, nullptr);
  runFor(0.2);
  ASSERT_GT(m_fakeCore->frameCount(), 0);

  m_controller.m_paused = true;
  runFor(0.2);
  const auto framesWhilePaused = m_fakeCore->frameCount();
  m_emulationService->resetGame();
  runFor(0.05);

  EXPECT_EQ(m_fakeCore->frameCount(), 0) << "the reset ran while paused";
  EXPECT_GT(framesWhilePaused, 0);
}

TEST_F(EmulationLoopTest, AFrameStepWhilePausedRunsExactlyOne) {
  auto *instance = loadGame();
  ASSERT_NE(instance, nullptr);
  runFor(0.2);
  m_controller.m_paused = true;
  runFor(0.1);
  const auto before = m_fakeCore->frameCount();

  instance->submitCommand({.type = EmulatorCommandType::RunFrame});
  runFor(0.05);

  EXPECT_EQ(m_fakeCore->frameCount(), before + 1);
}

TEST_F(EmulationLoopTest, DoubleSpeedRunsTwoFramesPerTick) {
  ASSERT_NE(loadGame(), nullptr);
  m_controller.m_multiplier = 2.0F;

  runFor(1.0);

  EXPECT_NEAR(m_fakeCore->frameCount(), 120, 4);
}

TEST_F(EmulationLoopTest, HalfSpeedRunsEveryOtherTick) {
  ASSERT_NE(loadGame(), nullptr);
  m_controller.m_multiplier = 0.5F;

  runFor(1.0);

  EXPECT_NEAR(m_fakeCore->frameCount(), 30, 2);
}

TEST_F(EmulationLoopTest, NothingRunsWithoutAGame) {
  runFor(0.5);

  EXPECT_EQ(m_passes, 0);
}

TEST_F(EmulationLoopTest, AsksForAPassWhenNothingPresents) {
  ASSERT_NE(loadGame(), nullptr);

  runFor(1.0, false);

  EXPECT_EQ(m_fakeCore->frameCount(), 0);
  EXPECT_GE(m_passes, 3);
}

} // namespace firelight::emulation

#include <firelight/audio/audio_rate_controller.hpp>

#include <gtest/gtest.h>

namespace {
constexpr int CAPACITY = 16384;

// Feed the same occupancy enough times to fill the rolling average window
int settle(AudioRateController &controller, int usedBytes) {
  int delta = 0;
  for (int i = 0; i < 15; ++i) {
    delta = controller.computeCompensation(usedBytes, CAPACITY);
  }
  return delta;
}
} // namespace

TEST(AudioRateControllerTest, DropsSamplesWhenBufferFull) {
  AudioRateController controller;
  EXPECT_LT(settle(controller, CAPACITY), 0);
}

TEST(AudioRateControllerTest, AddsSamplesWhenBufferEmpty) {
  AudioRateController controller;
  EXPECT_GT(settle(controller, 0), 0);
}

TEST(AudioRateControllerTest, NoCompensationNearTarget) {
  AudioRateController controller;
  EXPECT_EQ(settle(controller, CAPACITY / 2), 0);
}

TEST(AudioRateControllerTest, ZeroCapacityIsSafe) {
  AudioRateController controller;
  EXPECT_EQ(controller.computeCompensation(100, 0), 0);
}

TEST(AudioRateControllerTest, ResetClearsWindow) {
  AudioRateController controller;
  ASSERT_LT(settle(controller, CAPACITY), 0);

  controller.reset();
  EXPECT_GT(controller.computeCompensation(0, CAPACITY), 0);
}

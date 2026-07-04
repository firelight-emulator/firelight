#include <firelight/audio/audio_rate_controller.hpp>

#include <gtest/gtest.h>

namespace {
constexpr int kCapacity = 16384;

// Feed the same occupancy enough times to fill the moving-average window.
int settle(AudioRateController &controller, int usedBytes) {
  int delta = 0;
  for (int i = 0; i < 15; ++i) {
    delta = controller.computeCompensation(usedBytes, kCapacity);
  }
  return delta;
}
} // namespace

TEST(AudioRateControllerTest, DropsSamplesWhenBufferFull) {
  AudioRateController controller;
  EXPECT_LT(settle(controller, kCapacity), 0); // >target -> drop samples
}

TEST(AudioRateControllerTest, AddsSamplesWhenBufferEmpty) {
  AudioRateController controller;
  EXPECT_GT(settle(controller, 0), 0); // <target -> add samples
}

TEST(AudioRateControllerTest, NoCompensationNearTarget) {
  AudioRateController controller;
  EXPECT_EQ(settle(controller, kCapacity / 2), 0); // ~50% full -> leave alone
}

TEST(AudioRateControllerTest, ZeroCapacityIsSafe) {
  AudioRateController controller;
  EXPECT_EQ(controller.computeCompensation(100, 0), 0);
}

TEST(AudioRateControllerTest, ResetClearsWindow) {
  AudioRateController controller;
  ASSERT_LT(settle(controller, kCapacity), 0); // full -> negative
  controller.reset();
  // A fresh window: an empty buffer now reads as low from the first sample.
  EXPECT_GT(controller.computeCompensation(0, kCapacity), 0);
}

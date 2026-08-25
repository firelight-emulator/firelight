#include <firelight/audio/audio_rate_controller.hpp>

#include <gtest/gtest.h>

namespace {
constexpr int CAPACITY = 16384;
constexpr int FRAMES_PER_CALL = 800;

// Feed the same occupancy until the smoothing has caught up with it
double settle(AudioRateController &controller, int usedBytes) {
  double delta = 0.0;
  for (int i = 0; i < 200; ++i) {
    delta = controller.computeCompensation(usedBytes, CAPACITY, FRAMES_PER_CALL);
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
  EXPECT_EQ(settle(controller, CAPACITY / 2), 0.0);
}

TEST(AudioRateControllerTest, ZeroCapacityIsSafe) {
  AudioRateController controller;
  EXPECT_EQ(controller.computeCompensation(100, 0, FRAMES_PER_CALL), 0.0);
}

TEST(AudioRateControllerTest, ResetClearsWindow) {
  AudioRateController controller;
  ASSERT_LT(settle(controller, CAPACITY), 0);

  controller.reset();
  EXPECT_GT(controller.computeCompensation(0, CAPACITY, FRAMES_PER_CALL), 0);
}

// TODO
// How far the smoothing reaches has to be a span of sound, not a number of calls. A core handing over
// eight small batches a frame must get the same window as one handing over a single large one —
// otherwise the controller's reach depends on a choice the core made, which has nothing to do with
// the drift it is supposed to be tracking
TEST(AudioRateControllerTest, SmoothingSpansAudioRatherThanCallbacks) {
  const auto audioToConverge = [](const int framesPerCall) {
    AudioRateController controller;

    // Hold it at half full until it believes that, then drop the buffer to empty and count how much
    // audio has to pass before it has swung most of the way to a full correction
    for (auto i = 0; i < 5000; ++i) {
      controller.computeCompensation(CAPACITY / 2, CAPACITY, framesPerCall);
    }

    double audioFrames = 0.0;
    for (auto i = 0; i < 100000; ++i) {
      const auto ratio = controller.computeCompensation(0, CAPACITY, framesPerCall);
      audioFrames += framesPerCall;

      // One time constant of an exponential is 63% of the way there
      if (ratio >= AudioRateController::MAX_CORRECTION * 0.63) {
        break;
      }
    }

    return audioFrames;
  };

  const auto inLargeCalls = audioToConverge(800);
  const auto inSmallCalls = audioToConverge(100);

  EXPECT_NEAR(inSmallCalls, inLargeCalls, inLargeCalls * 0.2)
      << "took " << inSmallCalls << " frames of audio in small calls but " << inLargeCalls << " in large";
}

// TODO
// The window is a multiple of the sink's capacity, so a smaller buffer gets a proportionately quicker
// servo — the two are coupled, and a buffer that holds less has less time to spare
TEST(AudioRateControllerTest, SmoothingScalesWithTheBuffer) {
  const auto audioToConverge = [](const int capacity) {
    AudioRateController controller;

    for (auto i = 0; i < 5000; ++i) {
      controller.computeCompensation(capacity / 2, capacity, FRAMES_PER_CALL);
    }

    double audioFrames = 0.0;
    for (auto i = 0; i < 100000; ++i) {
      const auto ratio = controller.computeCompensation(0, capacity, FRAMES_PER_CALL);
      audioFrames += FRAMES_PER_CALL;

      if (ratio >= AudioRateController::MAX_CORRECTION * 0.63) {
        break;
      }
    }

    return audioFrames;
  };

  EXPECT_NEAR(audioToConverge(CAPACITY * 2), audioToConverge(CAPACITY) * 2.0, audioToConverge(CAPACITY) * 0.3);
}

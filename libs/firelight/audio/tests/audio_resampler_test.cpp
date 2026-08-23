#include <firelight/audio/audio_resampler.hpp>

#include <gtest/gtest.h>
#include <vector>

namespace {
constexpr int DEVICE_RATE = 48000;
constexpr int CD_RATE = 44100;
constexpr int GAME_BOY_RATE = 32768;
constexpr int N64_RATE = 33600;

// Interleaved stereo silence; only the frame count matters to these tests
std::vector<int16_t> silence(const size_t frameCount) { return std::vector<int16_t>(frameCount * 2, 0); }

// Frames produced by one process() call
size_t convert(AudioResampler &resampler, const size_t frameCount, const double compensationRatio = 0.0) {
  const auto input = silence(frameCount);
  return resampler.process(input.data(), frameCount, compensationRatio).size() / 2;
}

// The rate that asks for `samples` more or fewer out of a call producing `outOf` of them
double ratioFor(const double samples, const double outOf) { return samples / outOf; }

// Runs enough audio through to get past the resampler's initial delay, so a
// later measurement reflects the steady state
void settle(AudioResampler &resampler, const size_t frameCount) {
  for (auto i = 0; i < 4; ++i) {
    convert(resampler, frameCount);
  }
}
} // namespace

TEST(AudioResamplerTest, StartsUninitialized) {
  AudioResampler resampler;
  EXPECT_FALSE(resampler.isInitialized());
}

TEST(AudioResamplerTest, ProcessesNothingBeforeInitialize) {
  AudioResampler resampler;
  const auto input = silence(512);

  EXPECT_TRUE(resampler.process(input.data(), 512, 0).empty());
}

TEST(AudioResamplerTest, InitializeMarksReady) {
  AudioResampler resampler;
  resampler.initialize(DEVICE_RATE);

  EXPECT_TRUE(resampler.isInitialized());
}

TEST(AudioResamplerTest, PassthroughRateKeepsFrameCount) {
  AudioResampler resampler;
  resampler.initialize(DEVICE_RATE);
  settle(resampler, 1024);

  EXPECT_NEAR(static_cast<double>(convert(resampler, 1024)), 1024.0, 8.0);
}

TEST(AudioResamplerTest, UpsamplesCdRateToDeviceRate) {
  AudioResampler resampler;
  resampler.initialize(CD_RATE, DEVICE_RATE);
  settle(resampler, 4410);

  // 4410 frames at 44100 is 100ms, which is 4800 frames at 48000
  EXPECT_NEAR(static_cast<double>(convert(resampler, 4410)), 4800.0, 16.0);
}

TEST(AudioResamplerTest, RatioAboveOneProducesFewerFrames) {
  AudioResampler resampler;
  resampler.setPlaybackRateRatio(2.0);
  resampler.initialize(DEVICE_RATE, DEVICE_RATE);
  settle(resampler, 1024);

  EXPECT_NEAR(static_cast<double>(convert(resampler, 1024)), 512.0, 8.0);
}

TEST(AudioResamplerTest, RatioBelowOneProducesMoreFrames) {
  AudioResampler resampler;
  resampler.setPlaybackRateRatio(0.5);
  resampler.initialize(DEVICE_RATE, DEVICE_RATE);
  settle(resampler, 1024);

  EXPECT_NEAR(static_cast<double>(convert(resampler, 1024)), 2048.0, 16.0);
}

TEST(AudioResamplerTest, RatioChangeAfterInitializeRebuilds) {
  AudioResampler resampler;
  resampler.initialize(DEVICE_RATE, DEVICE_RATE);
  settle(resampler, 1024);
  ASSERT_NEAR(static_cast<double>(convert(resampler, 1024)), 1024.0, 8.0);

  resampler.setPlaybackRateRatio(2.0);
  settle(resampler, 1024);

  EXPECT_NEAR(static_cast<double>(convert(resampler, 1024)), 512.0, 8.0);
}

TEST(AudioResamplerTest, NonPositiveRatioIsClampedToOne) {
  AudioResampler resampler;
  resampler.setPlaybackRateRatio(-1.0);
  resampler.initialize(DEVICE_RATE, DEVICE_RATE);
  settle(resampler, 1024);

  EXPECT_NEAR(static_cast<double>(convert(resampler, 1024)), 1024.0, 8.0);
}

// Cores do not hand over even batches — mupen alternates roughly 500 frames and 30 — and a
// correction meaning "this many samples" lands on the small one as a swing of over ten percent. As a
// rate it is the same correction on both
TEST(AudioResamplerTest, ASmallBatchGetsAProportionateCorrection) {
  AudioResampler resampler;
  resampler.initialize(N64_RATE, DEVICE_RATE);

  const auto pull = ratioFor(-5, 800);

  for (auto i = 0; i < 20; ++i) {
    convert(resampler, 500, pull);
    convert(resampler, 30, pull);
  }

  // 500 frames at 33600 is 714 at 48000, and 30 frames is 43
  EXPECT_NEAR(static_cast<double>(convert(resampler, 500, pull)), 714.0, 8.0);
  EXPECT_NEAR(static_cast<double>(convert(resampler, 30, pull)), 43.0, 3.0);
}

// The shape this went wrong in: a core rate that divides into neither the device rate nor the
// callback size. Every call has to carry its own share, rather than alternating between one that
// comes up short and one that dumps the arrears
TEST(AudioResamplerTest, UpsamplingIsSteadyFromCallToCall) {
  AudioResampler resampler;
  resampler.initialize(GAME_BOY_RATE, DEVICE_RATE);
  settle(resampler, 549);

  // 549 frames at 32768 is 804 frames at 48000
  for (auto i = 0; i < 8; ++i) {
    EXPECT_NEAR(static_cast<double>(convert(resampler, 549)), 804.0, 8.0);
  }
}

// A positive delta spreads that many extra samples over the call's window
TEST(AudioResamplerTest, PositiveCompensationDeltaRaisesOutputCount) {
  AudioResampler resampler;
  resampler.initialize(DEVICE_RATE, DEVICE_RATE);
  settle(resampler, 1024);

  const auto baseline = convert(resampler, 1024, 0);
  settle(resampler, 1024);
  const auto compensated = convert(resampler, 1024, ratioFor(200, 1024));

  EXPECT_GT(compensated, baseline);
  EXPECT_NEAR(static_cast<double>(compensated), static_cast<double>(baseline) + 200.0, 8.0);
}

// The direction AudioManager depends on: AudioRateController returns a negative
// delta when the sink buffer is too full, and that has to drain it
TEST(AudioResamplerTest, NegativeCompensationDeltaReducesOutputCount) {
  AudioResampler resampler;
  resampler.initialize(DEVICE_RATE, DEVICE_RATE);
  settle(resampler, 1024);

  const auto baseline = convert(resampler, 1024, 0);
  settle(resampler, 1024);
  const auto compensated = convert(resampler, 1024, ratioFor(-200, 1024));

  EXPECT_LT(compensated, baseline);
}

// A buffer far smaller than one callback's worth still converts rather than being held back
TEST(AudioResamplerTest, HandlesTinyBuffers) {
  AudioResampler resampler;
  resampler.initialize(DEVICE_RATE, DEVICE_RATE);
  settle(resampler, 64);

  EXPECT_NEAR(static_cast<double>(convert(resampler, 64)), 64.0, 8.0);
}

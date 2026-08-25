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

// TODO
// Over a long run the output has to keep pace with the input whatever the batch sizes. A call that
// comes up a fraction of a sample short every time is a rate error, not a rounding detail — and it is
// the one thing no single-call assertion above can see
TEST(AudioResamplerTest, ThroughputHoldsOverManyCalls) {
  AudioResampler resampler;
  resampler.initialize(N64_RATE, DEVICE_RATE);

  size_t inFrames = 0;
  size_t outFrames = 0;

  for (auto i = 0; i < 2000; ++i) {
    const size_t batch = i % 2 == 0 ? 500 : 30;
    inFrames += batch;
    outFrames += convert(resampler, batch);
  }

  const auto expected = static_cast<double>(inFrames) * DEVICE_RATE / N64_RATE;
  EXPECT_NEAR(static_cast<double>(outFrames) / expected, 1.0, 0.0005)
      << outFrames << " frames out for " << inFrames << " in; expected " << expected;
}

TEST(AudioResamplerTest, ThroughputHoldsForEvenBatches) {
  AudioResampler resampler;
  resampler.initialize(N64_RATE, DEVICE_RATE);

  size_t outFrames = 0;
  constexpr size_t BATCH = 560;
  constexpr int CALLS = 2000;

  for (auto i = 0; i < CALLS; ++i) {
    outFrames += convert(resampler, BATCH);
  }

  const auto expected = static_cast<double>(BATCH * CALLS) * DEVICE_RATE / N64_RATE;
  EXPECT_NEAR(static_cast<double>(outFrames) / expected, 1.0, 0.0005)
      << outFrames << " frames out; expected " << expected;
}

// TODO
// Open loop, because a closed one hides this: if only half the correction lands, the controller
// simply sits at twice the error and the amount that lands still matches what is physically needed.
// Handed a fixed rate against alternating batches, what comes out has to match what was asked for —
// a batch whose share rounds to nothing must not silently forfeit it
TEST(AudioResamplerTest, SmallBatchesCarryTheirShareOfTheCorrection) {
  AudioResampler resampler;
  resampler.initialize(N64_RATE, DEVICE_RATE);

  constexpr double RATE = 0.002;
  size_t inFrames = 0;
  size_t outFrames = 0;

  for (auto i = 0; i < 2000; ++i) {
    const size_t batch = i % 2 == 0 ? 500 : 30;
    inFrames += batch;
    outFrames += convert(resampler, batch, RATE);
  }

  const auto nominal = static_cast<double>(inFrames) * DEVICE_RATE / N64_RATE;
  const auto applied = (static_cast<double>(outFrames) - nominal) / nominal;

  EXPECT_NEAR(applied, RATE, RATE * 0.25) << "asked for " << RATE << ", " << applied << " landed";
}

TEST(AudioResamplerTest, SmallBatchesCarryTheirShareWhenShortening) {
  AudioResampler resampler;
  resampler.initialize(N64_RATE, DEVICE_RATE);

  constexpr double RATE = -0.002;
  size_t inFrames = 0;
  size_t outFrames = 0;

  for (auto i = 0; i < 2000; ++i) {
    const size_t batch = i % 2 == 0 ? 500 : 30;
    inFrames += batch;
    outFrames += convert(resampler, batch, RATE);
  }

  const auto nominal = static_cast<double>(inFrames) * DEVICE_RATE / N64_RATE;
  const auto applied = (static_cast<double>(outFrames) - nominal) / nominal;

  EXPECT_NEAR(applied, RATE, std::abs(RATE) * 0.25) << "asked for " << RATE << ", " << applied << " landed";
}

// TODO
// The case both throughput tests above miss, because each of their patterns contains a batch large
// enough to drain whatever the resampler held back. A core handing over the SAME small batch every
// time never offers that slack, so anything withheld is withheld for good — a rate error, not a delay
TEST(AudioResamplerTest, ThroughputHoldsForAConstantSmallBatch) {
  for (const size_t batch : {size_t(30), size_t(32), size_t(34), size_t(64)}) {
    AudioResampler resampler;
    resampler.initialize(N64_RATE, DEVICE_RATE);

    size_t outFrames = 0;
    constexpr int CALLS = 20000;

    for (auto i = 0; i < CALLS; ++i) {
      outFrames += convert(resampler, batch);
    }

    const auto expected = static_cast<double>(batch * CALLS) * DEVICE_RATE / N64_RATE;
    EXPECT_NEAR(static_cast<double>(outFrames) / expected, 1.0, 0.0005)
        << "batch " << batch << ": " << outFrames << " frames out, expected " << expected << " ("
        << (static_cast<double>(outFrames) / expected - 1.0) * 1e6 << " ppm)";
  }
}

TEST(AudioResamplerTest, ThroughputHoldsForAConstantSmallBatchUpsampling) {
  AudioResampler resampler;
  resampler.initialize(CD_RATE, DEVICE_RATE);

  size_t outFrames = 0;
  constexpr size_t BATCH = 16;
  constexpr int CALLS = 20000;

  for (auto i = 0; i < CALLS; ++i) {
    outFrames += convert(resampler, BATCH);
  }

  const auto expected = static_cast<double>(BATCH * CALLS) * DEVICE_RATE / CD_RATE;
  EXPECT_NEAR(static_cast<double>(outFrames) / expected, 1.0, 0.0005)
      << (static_cast<double>(outFrames) / expected - 1.0) * 1e6 << " ppm";
}

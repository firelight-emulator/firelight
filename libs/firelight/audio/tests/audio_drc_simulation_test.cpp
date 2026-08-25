#include <firelight/audio/audio_rate_controller.hpp>
#include <firelight/audio/audio_resampler.hpp>

#include <algorithm>
#include <cmath>
#include <gtest/gtest.h>
#include <numeric>
#include <vector>

// TODO
// Drives the rate controller against a model of the things it actually has to cope with: a device
// whose true rate differs from its nominal one and which only reports its progress in coarse steps,
// a core that hands over uneven batches, and an actuator that can only move whole samples.
//
// The numbers come from measurement on a MacBook Pro: the sink reports drain in 10.67 ms steps and
// nothing else, and the device's true rate sits about 9 parts per million off nominal.
namespace {

constexpr int BYTES_PER_FRAME = 4; // stereo int16

// TODO
// What the controller is up against for one run
struct SimConfig {
  /** Fractional difference between the device's real rate and its nominal one */
  double deviceRateError = 0.0;
  /** How coarsely the sink reports what it has consumed */
  double reportPeriodSecs = 10.667e-3;
  double deviceRate = 48000.0;
  int capacityFrames = 3072; // the shipping 64 ms buffer at 48 kHz
  double contentFps = 60.0988;
  /** Input frames the core hands over on each successive emulated frame, cycled */
  std::vector<int> framePattern = {534};
};

// TODO
// What one run produced, sampled once per emulated frame
struct SimResult {
  std::vector<double> occupancy;
  std::vector<double> requestedRatio;
  // TODO
  // Running totals rather than per-call deltas: the resampler holds back whatever a call's window
  // could not fit and delivers it on a later one, so only the accumulated figures mean anything
  std::vector<double> producedTotal;
  std::vector<double> nominalTotal;
  int underruns = 0;
  int overruns = 0;
};

// TODO
// The correction that actually landed over the tail of a run, as a fraction of what was nominally due
double appliedOver(const SimResult &result, const double fraction = 0.5) {
  const auto from = static_cast<size_t>(static_cast<double>(result.producedTotal.size()) * (1.0 - fraction));
  const auto produced = result.producedTotal.back() - result.producedTotal[from];
  const auto nominal = result.nominalTotal.back() - result.nominalTotal[from];

  return nominal > 0.0 ? (produced - nominal) / nominal : 0.0;
}

double mean(const std::vector<double> &values) {
  if (values.empty()) {
    return 0.0;
  }

  return std::accumulate(values.begin(), values.end(), 0.0) / static_cast<double>(values.size());
}

// TODO
// The tail of a run, once any startup transient has passed
std::vector<double> tail(const std::vector<double> &values, const double fraction = 0.5) {
  const auto from = static_cast<size_t>(static_cast<double>(values.size()) * (1.0 - fraction));
  return {values.begin() + static_cast<long>(from), values.end()};
}

// TODO
// How much a series swings between alternate entries. For a core that alternates its delivery every
// frame this is the size of any wobble locked to that pattern, which is the thing that would be heard
double alternatingSwing(const std::vector<double> &values) {
  std::vector<double> even;
  std::vector<double> odd;

  for (size_t i = 0; i < values.size(); ++i) {
    (i % 2 == 0 ? even : odd).push_back(values[i]);
  }

  return std::abs(mean(even) - mean(odd));
}

// TODO
// Where the buffer should settle, which is not half full, for two reasons that both have to hold.
//
// A proportional correction only exists while there is an error to be proportional to, so cancelling
// a device running e fast means sitting e/MAX_CORRECTION of the way off target. And the sink reports
// what it has consumed only in whole periods, so between reports the reading overstates occupancy by
// up to a period — half of one on average — and the controller holds the true level that much lower.
// The second term is what makes a small buffer sit further off centre: the same period is a larger
// share of it
double expectedOccupancy(const SimConfig &config) {
  const auto fromDrift = config.deviceRateError / AudioRateController::MAX_CORRECTION * 0.5;
  const auto fromReportingLag = 0.5 * (config.reportPeriodSecs * config.deviceRate) / config.capacityFrames;

  return 0.5 - fromDrift - fromReportingLag;
}

SimResult simulate(AudioRateController &controller, const SimConfig &config, const double seconds) {
  SimResult result;

  const auto capacityBytes = config.capacityFrames * BYTES_PER_FRAME;
  const auto patternMean =
      std::accumulate(config.framePattern.begin(), config.framePattern.end(), 0.0) / config.framePattern.size();
  // The core's rate follows from what it hands over, and the frame rate follows from the rate, so
  // the model stays exactly self-consistent whatever pattern it is given
  const auto coreRate = std::lround(patternMean * config.contentFps);
  const auto contentFps = static_cast<double>(coreRate) / patternMean;

  // The real resampler, so what the tests measure is the correction that actually lands rather than
  // a second implementation of the arithmetic
  AudioResampler resampler;
  resampler.initialize(static_cast<int>(coreRate), static_cast<int>(config.deviceRate));

  const auto largestBatch = *std::max_element(config.framePattern.begin(), config.framePattern.end());
  const std::vector<int16_t> input(static_cast<size_t>(largestBatch) * 2, 0);

  const auto trueDeviceRate = config.deviceRate * (1.0 + config.deviceRateError);
  const auto reportStepFrames = config.reportPeriodSecs * config.deviceRate;
  const auto framePeriod = 1.0 / contentFps;

  // AudioManager primes the sink to about half full before it starts, so the run begins where the
  // real one does rather than converging up from empty
  double written = config.capacityFrames / 2.0;
  double consumed = 0.0;
  double producedRunning = 0.0;
  double nominalRunning = 0.0;
  const auto totalFrames = static_cast<int>(seconds * contentFps);

  for (auto frame = 0; frame < totalFrames; ++frame) {
    // The sink only ever admits to whole steps of progress, which is what the controller sees
    const auto reportedConsumed = std::floor(consumed / reportStepFrames) * reportStepFrames;
    const auto reportedUsed = std::clamp(written - reportedConsumed, 0.0, static_cast<double>(config.capacityFrames));

    const auto inputFrames = config.framePattern[frame % config.framePattern.size()];
    const auto nominalOut = static_cast<double>(inputFrames) * config.deviceRate / static_cast<double>(coreRate);

    const auto ratio = controller.computeCompensation(static_cast<int>(reportedUsed * BYTES_PER_FRAME), capacityBytes,
                                                      static_cast<int>(nominalOut));
    const auto produced = static_cast<double>(resampler.process(input.data(), inputFrames, ratio).size()) / 2.0;

    written += produced;
    producedRunning += produced;
    nominalRunning += nominalOut;
    consumed += trueDeviceRate * framePeriod;

    const auto trueUsed = written - consumed;
    if (trueUsed <= 0.0) {
      result.underruns++;
      // A real sink cannot hold a negative amount; it plays silence and carries on
      consumed = written;
    } else if (trueUsed > config.capacityFrames) {
      result.overruns++;
      written = consumed + config.capacityFrames;
    }

    result.occupancy.push_back(std::clamp(written - consumed, 0.0, static_cast<double>(config.capacityFrames)) /
                               config.capacityFrames);
    result.requestedRatio.push_back(ratio);
    result.producedTotal.push_back(producedRunning);
    result.nominalTotal.push_back(nominalRunning);
  }

  return result;
}

} // namespace

// TODO
// Bug 2: a correction that only engages once the buffer is far from target lets a small persistent
// error run unopposed, so the buffer crosses the dead zone, takes one kick, and drifts back
TEST(AudioDrcSimulationTest, SettlesInsteadOfSawtoothingOnASlowDevice) {
  AudioRateController controller;
  SimConfig config;
  config.deviceRateError = -9e-6; // the 9 ppm actually measured on this machine

  // An hour, because 9 ppm is only 0.43 samples a second — it takes a real session to matter, and
  // "corrects slow drift over long sessions" is precisely what the setting promises
  const auto result = simulate(controller, config, 3600.0);

  const auto settled = tail(result.occupancy, 0.6);
  const auto low = *std::min_element(settled.begin(), settled.end());
  const auto high = *std::max_element(settled.begin(), settled.end());

  EXPECT_NEAR(mean(settled), expectedOccupancy(config), 0.02)
      << "should hold where the gain and the reporting lag put it";
  EXPECT_LT(high - low, 0.1) << "occupancy swept " << (high - low) * 100 << "% of the buffer";
}

// TODO
// Bug 3: the controller must not chase the core's own delivery rhythm. Mupen alternates a large batch
// one frame and a small one the next, so occupancy genuinely rises and falls every other frame — real
// movement, but not drift, and correcting for it puts a wobble on everything the game plays
TEST(AudioDrcSimulationTest, DoesNotFollowTheCoresDeliveryPattern) {
  AudioRateController controller;
  SimConfig config;
  config.framePattern = {500, 30};

  const auto result = simulate(controller, config, 60.0);

  EXPECT_LT(alternatingSwing(tail(result.requestedRatio)), 1e-4)
      << "the requested rate is swinging in step with the core's batches";
}

// TODO
// Crystals drift by tens of parts per million, which is the case this has to be good at
TEST(AudioDrcSimulationTest, HoldsTheBufferAcrossRealisticDrift) {
  for (const auto error : {-1e-4, -9e-6, 0.0, 9e-6, 1e-4}) {
    AudioRateController controller;
    SimConfig config;
    config.deviceRateError = error;

    const auto result = simulate(controller, config, 90.0);

    EXPECT_EQ(result.underruns, 0) << "at " << error;
    EXPECT_EQ(result.overruns, 0) << "at " << error;
    EXPECT_NEAR(mean(tail(result.occupancy)), expectedOccupancy(config), 0.02) << "at " << error;
  }
}

// TODO
// A rate wrong by tenths of a percent is a misreported rate rather than drift, and correcting one
// means sitting away from half full: a proportional correction only exists while there is an error to
// be proportional to. Holding a safe distance from both ends is what matters, not sitting on target
TEST(AudioDrcSimulationTest, SurvivesAMisreportedRateWithoutRunningDry) {
  for (const auto error : {-2e-3, 2e-3}) {
    AudioRateController controller;
    SimConfig config;
    config.deviceRateError = error;

    const auto result = simulate(controller, config, 300.0);

    const auto settled = tail(result.occupancy, 0.7);
    const auto low = *std::min_element(settled.begin(), settled.end());
    const auto high = *std::max_element(settled.begin(), settled.end());

    EXPECT_EQ(result.underruns, 0) << "at " << error;
    EXPECT_EQ(result.overruns, 0) << "at " << error;
    EXPECT_GT(low, 0.15) << "at " << error << ", came within " << low * 100 << "% of running dry";
    EXPECT_LT(high, 0.85) << "at " << error << ", reached " << high * 100 << "% full";
    // The offset is what produces the correction, and it should be the size the gain implies
    EXPECT_NEAR(mean(settled), expectedOccupancy(config), 0.03) << "at " << error;
  }
}

// TODO
// The bound alone guarantees nothing audible — what matters is that against the drift a real crystal
// has, the correction sits far inside it and does not jump about. A steady offset is inaudible; one
// that moves is a warble
TEST(AudioDrcSimulationTest, KeepsTheCorrectionSmallAndSteady) {
  AudioRateController controller;
  SimConfig config;
  config.deviceRateError = 9e-6;

  const auto result = simulate(controller, config, 90.0);
  const auto settled = tail(result.requestedRatio);

  for (const auto ratio : settled) {
    EXPECT_LT(std::abs(ratio), AudioRateController::MAX_CORRECTION * 0.2)
        << "using " << std::abs(ratio) / AudioRateController::MAX_CORRECTION * 100 << "% of the available range for "
        << "drift of nine parts per million";
  }

  double largestJump = 0.0;
  for (size_t i = 1; i < settled.size(); ++i) {
    largestJump = std::max(largestJump, std::abs(settled[i] - settled[i - 1]));
  }

  // A tenth of a cent per step. Pitch is perceived logarithmically and a cent is already generous for
  // program material, so this bounds the movement well below anything that could be heard as warble —
  // while a controller stepping between fixed levels would move tens of times further
  constexpr double A_TENTH_OF_A_CENT = 1e-4;
  EXPECT_LT(largestJump, A_TENTH_OF_A_CENT)
      << "the correction moved by " << largestJump << " between consecutive calls, which is "
      << 1200.0 * std::log2(1.0 + largestJump) << " cents";
}

// TODO
// A coarser sink should cost precision, not stability
TEST(AudioDrcSimulationTest, ToleratesWhateverGranularityTheSinkReports) {
  for (const auto periodMs : {2.67, 10.667, 21.0}) {
    AudioRateController controller;
    SimConfig config;
    config.reportPeriodSecs = periodMs / 1000.0;
    config.deviceRateError = 5e-4;

    const auto result = simulate(controller, config, 90.0);

    EXPECT_EQ(result.underruns, 0) << "at " << periodMs << "ms";
    EXPECT_EQ(result.overruns, 0) << "at " << periodMs << "ms";
    EXPECT_NEAR(mean(tail(result.occupancy)), expectedOccupancy(config), 0.02) << "at " << periodMs << "ms";
  }
}

// TODO
// Buffer size is a latency the player feels, and it is theirs to choose — but it also sets how far
// the controller's smoothing reaches, since the window is a multiple of the capacity. Every size on
// offer has to hold, not just the one that ships
TEST(AudioDrcSimulationTest, HoldsAtEveryBufferSizeOnOffer) {
  constexpr int RATE = 48000;

  for (const auto latencyMs : {32, 64, 128, 256}) {
    AudioRateController controller;
    SimConfig config;
    config.capacityFrames = latencyMs * RATE / 1000;
    config.deviceRateError = 1e-4;

    const auto result = simulate(controller, config, 120.0);

    EXPECT_EQ(result.underruns, 0) << "at " << latencyMs << "ms";
    EXPECT_EQ(result.overruns, 0) << "at " << latencyMs << "ms";
    EXPECT_NEAR(mean(tail(result.occupancy)), expectedOccupancy(config), 0.02) << "at " << latencyMs << "ms";
    // However far off centre it settles, it has to stay clear of both ends
    EXPECT_GT(mean(tail(result.occupancy)), 0.2) << "at " << latencyMs << "ms";
  }
}

// TODO
// The worst case for a small buffer: a core that alternates a large batch with a small one, so
// occupancy genuinely swings every other frame, against the least latency on offer
TEST(AudioDrcSimulationTest, HoldsTheSmallestBufferAgainstUnevenDelivery) {
  AudioRateController controller;
  SimConfig config;
  config.capacityFrames = 32 * 48000 / 1000;
  config.framePattern = {500, 30};
  config.deviceRateError = 5e-4;

  const auto result = simulate(controller, config, 120.0);

  EXPECT_EQ(result.underruns, 0);
  EXPECT_EQ(result.overruns, 0);
  EXPECT_LT(alternatingSwing(tail(result.requestedRatio)), 1e-4)
      << "the correction is swinging in step with the core's batches";
}

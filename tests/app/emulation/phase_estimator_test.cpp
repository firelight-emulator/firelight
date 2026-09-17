// TODO: NEEDS REVIEW
#include <cmath>
#include <cstdint>
#include <emulation/phase_estimator.hpp>
#include <gtest/gtest.h>

using firelight::emulation::PhaseEstimator;

namespace {
constexpr int64_t SECOND_NS = 1000000000;
constexpr double REPORTED_HZ = 60.0;
constexpr double REAL_HZ = 59.959;
constexpr int64_t NOMINAL_NS = static_cast<int64_t>(SECOND_NS / REPORTED_HZ);
constexpr double REAL_PERIOD_NS = SECOND_NS / REAL_HZ;

// A small deterministic generator, so the jitter is the same on every run
struct Noise {
  uint64_t state = 0x9E3779B97F4A7C15ULL;

  // Uniform in [-1, 1)
  double next() {
    state = state * 6364136223846793005ULL + 1442695040888963407ULL;
    return static_cast<double>(state >> 11) / static_cast<double>(1ULL << 53) * 2.0 - 1.0;
  }
};

/**
 * Feeds presents on a grid at the real rate with the given jitter, returning the instant of the last
 * present's ideal grid point
 */
int64_t feedTrain(PhaseEstimator &estimator, const int presents, const double jitterNs, Noise &noise,
                  int64_t startNs = SECOND_NS) {
  auto idealNs = static_cast<double>(startNs);

  for (auto index = 0; index < presents; ++index) {
    estimator.notePresent(static_cast<int64_t>(idealNs + noise.next() * jitterNs));
    idealNs += REAL_PERIOD_NS;
  }

  return static_cast<int64_t>(idealNs - REAL_PERIOD_NS);
}
} // namespace

TEST(PhaseEstimatorTest, BeforeAnyPresentTheGridIsWhereverIsAsked) {
  PhaseEstimator estimator;
  estimator.configure(NOMINAL_NS);

  EXPECT_FALSE(estimator.isLocked());
  EXPECT_EQ(estimator.getPeriodNs(), NOMINAL_NS);
  EXPECT_EQ(estimator.getGridNs(12345), 12345);
}

TEST(PhaseEstimatorTest, LocksToAJitteredTrainThatIsSlowerThanReported) {
  PhaseEstimator estimator;
  estimator.configure(NOMINAL_NS);
  Noise noise;

  const auto lastIdealNs = feedTrain(estimator, 300, 500000.0, noise);

  EXPECT_TRUE(estimator.isLocked());
  const auto periodError = std::abs(static_cast<double>(estimator.getPeriodNs()) - REAL_PERIOD_NS) / REAL_PERIOD_NS;
  EXPECT_LT(periodError, 0.001);
  EXPECT_LT(std::abs(estimator.getGridNs(lastIdealNs) - lastIdealNs), 1000000);
}

TEST(PhaseEstimatorTest, MeasuresACleanTrainToTensOfPartsPerMillion) {
  PhaseEstimator estimator;
  estimator.configure(NOMINAL_NS);
  Noise noise;

  const auto lastIdealNs = feedTrain(estimator, 3600, 0.0, noise);

  const auto periodError = std::abs(static_cast<double>(estimator.getPeriodNs()) - REAL_PERIOD_NS) / REAL_PERIOD_NS;
  EXPECT_LT(periodError, 0.00002);
  EXPECT_LT(std::abs(estimator.getGridNs(lastIdealNs) - lastIdealNs), 20000);
}

TEST(PhaseEstimatorTest, PredictsTheNextRefreshFromTheGrid) {
  PhaseEstimator estimator;
  estimator.configure(NOMINAL_NS);
  Noise noise;

  const auto lastIdealNs = feedTrain(estimator, 600, 200000.0, noise);
  const auto nextIdealNs = lastIdealNs + static_cast<int64_t>(REAL_PERIOD_NS);

  EXPECT_LT(std::abs(estimator.getGridNs(nextIdealNs) - nextIdealNs), 300000);
}

TEST(PhaseEstimatorTest, AGapRestartsTheGridAtThePresentAndDropsTheLock) {
  PhaseEstimator estimator;
  estimator.configure(NOMINAL_NS);
  Noise noise;

  const auto lastIdealNs = feedTrain(estimator, 120, 0.0, noise);
  ASSERT_TRUE(estimator.isLocked());

  const auto lateNs = lastIdealNs + PhaseEstimator::RESYNC_NS + 50000000;
  estimator.notePresent(lateNs);

  EXPECT_FALSE(estimator.isLocked());
  EXPECT_EQ(estimator.getGridNs(lateNs), lateNs);
}

TEST(PhaseEstimatorTest, OneLatePresentMovesTheGridByAFractionOfItsLateness) {
  PhaseEstimator estimator;
  estimator.configure(NOMINAL_NS);
  Noise noise;

  const auto lastIdealNs = feedTrain(estimator, 600, 0.0, noise);
  const auto nextIdealNs = lastIdealNs + static_cast<int64_t>(REAL_PERIOD_NS);
  const auto before = estimator.getGridNs(nextIdealNs);

  constexpr int64_t LATE_NS = 8000000;
  estimator.notePresent(nextIdealNs + LATE_NS);

  const auto moved = estimator.getGridNs(nextIdealNs) - before;
  EXPECT_GT(moved, 0);
  EXPECT_LE(moved, static_cast<int64_t>(PhaseEstimator::PHASE_GAIN * LATE_NS) + 1000);
  EXPECT_TRUE(estimator.isLocked());
}

TEST(PhaseEstimatorTest, ASecondPresentInsideTheSameRefreshIsLeftOut) {
  PhaseEstimator estimator;
  estimator.configure(NOMINAL_NS);
  Noise noise;

  const auto lastIdealNs = feedTrain(estimator, 600, 0.0, noise);
  const auto periodBefore = estimator.getPeriodNs();
  const auto gridBefore = estimator.getGridNs(lastIdealNs);

  estimator.notePresent(lastIdealNs + 2000000);

  EXPECT_EQ(estimator.getPeriodNs(), periodBefore);
  EXPECT_EQ(estimator.getGridNs(lastIdealNs), gridBefore);
  EXPECT_TRUE(estimator.isLocked());
}

TEST(PhaseEstimatorTest, ThePeriodNeverStraysBeyondTheTolerance) {
  PhaseEstimator estimator;
  estimator.configure(NOMINAL_NS);
  auto nowNs = SECOND_NS;

  // A train 5 % slow than reported, well outside what the display could really be
  for (auto index = 0; index < 600; ++index) {
    estimator.notePresent(nowNs);
    nowNs += static_cast<int64_t>(NOMINAL_NS * 1.05);
  }

  const auto ceiling = static_cast<int64_t>(NOMINAL_NS * (1.0 + PhaseEstimator::PERIOD_TOLERANCE));
  EXPECT_LE(estimator.getPeriodNs(), ceiling);
}

// TODO
// Presents that arrive in pairs, a few milliseconds apart around every other refresh, are what a
// three-image swapchain hands over; they must never pass for a grid
TEST(PhaseEstimatorTest, DoesNotTrustPresentsArrivingInPairs) {
  PhaseEstimator estimator;
  estimator.configure(NOMINAL_NS);
  auto idealNs = static_cast<double>(SECOND_NS);

  for (auto index = 0; index < 600; ++index) {
    const auto offsetNs = index % 2 == 0 ? 2000000.0 : -2000000.0;
    estimator.notePresent(static_cast<int64_t>(idealNs + offsetNs));
    idealNs += REAL_PERIOD_NS;
  }

  EXPECT_FALSE(estimator.isLocked());
}

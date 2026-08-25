#include <emulation/refresh_counter.hpp>
#include <gtest/gtest.h>

#include <algorithm>
#include <iterator>
#include <numeric>
#include <vector>

namespace firelight::emulation {

namespace {
// A round period, so that a gap of 1.8 refreshes is exactly 1800000ns rather than a truncation of
// it. The counter carries fractions on purpose, and a period that cannot express the test's own
// gaps leaves a fraction of its own behind for it to carry
constexpr int64_t PERIOD_NS = 1000000LL;

// Runs a steady cadence and returns what each present was counted as
std::vector<int> countAll(RefreshCounter &counter, const std::vector<double> &gapsInRefreshes, const int ceiling) {
  std::vector<int> counted;

  for (const auto gap : gapsInRefreshes) {
    counted.push_back(counter.observe(static_cast<int64_t>(gap * PERIOD_NS), PERIOD_NS, ceiling));
  }

  return counted;
}

int total(const std::vector<int> &counted) { return std::accumulate(counted.begin(), counted.end(), 0); }
} // namespace

// A frame held for more refreshes than the ceiling used to be counted as the ceiling forever: the
// part left over grew without bound instead of being paid back, so 6 refreshes a frame ran at 4/6
// speed
TEST(RefreshCounterTest, CountsEveryRefreshHoweverLongAFrameIsHeld) {
  for (const auto perFrame : {1, 2, 3, 4, 5, 6, 8, 12}) {
    RefreshCounter counter;
    const auto counted = countAll(counter, std::vector<double>(400, perFrame), RefreshCounter::ceilingFor(perFrame));

    EXPECT_EQ(total(counted), perFrame * 400) << "at " << perFrame << " refreshes per frame";
  }
}

// A window that stopped drawing owes nothing for the time it wasn't. Carrying that time forward
// paid it back afterwards as a run of frames, which is the game running fast to catch up on a
// backlog nobody was waiting for
TEST(RefreshCounterTest, DoesNotBankTimeTheWindowSpentNotDrawing) {
  RefreshCounter counter;
  const auto ceiling = RefreshCounter::ceilingFor(2);

  std::vector<double> gaps(100, 2.0);
  gaps.push_back(600.0); // five seconds of nothing reaching the screen
  gaps.insert(gaps.end(), 600, 2.0);

  const auto counted = countAll(counter, gaps, ceiling);
  const std::vector<int> afterTheStall(counted.begin() + 101, counted.end());

  for (size_t i = 0; i < afterTheStall.size(); ++i) {
    EXPECT_EQ(afterTheStall[i], 2) << "present " << i << " after the stall";
  }
}

// The stall itself is still worth something, or the frame it interrupted is never due
TEST(RefreshCounterTest, ASingleLatePresentIsStillWorthTheRefreshesItCovers) {
  RefreshCounter counter;
  const auto ceiling = RefreshCounter::ceilingFor(2);

  EXPECT_EQ(counter.observe(2 * PERIOD_NS, PERIOD_NS, ceiling), 2);
  EXPECT_EQ(counter.observe(3 * PERIOD_NS, PERIOD_NS, ceiling), 3);
  EXPECT_EQ(counter.observe(2 * PERIOD_NS, PERIOD_NS, ceiling), 2);
}

// Rounding each gap on its own cannot go below one, so a long gap rounds up while the short gap
// after it cannot round down
TEST(RefreshCounterTest, CarriesThePartLeftOverInsteadOfRoundingIt) {
  RefreshCounter counter;
  const auto counted = countAll(counter, std::vector<double>(200, 1.5), RefreshCounter::MIN_CEILING);

  EXPECT_EQ(total(counted), 300);
}

TEST(RefreshCounterTest, NeitherGainsNorLosesRefreshesUnderJitter) {
  RefreshCounter counter;
  std::vector<double> gaps;

  for (int i = 0; i < 400; ++i) {
    gaps.push_back(i % 2 == 0 ? 1.8 : 2.2);
  }

  const auto counted = countAll(counter, gaps, RefreshCounter::ceilingFor(2));

  EXPECT_EQ(total(counted), 800);
}

// With presentation unlocked the presents are not refreshes at all and can arrive far faster than
// them. Counting one apiece overstates the refreshes either way; what must not happen is the part
// left over running away, so that the count is still wrong long after presentation locks again
TEST(RefreshCounterTest, PresentsFasterThanRefreshesCannotRunTheCarryAway) {
  RefreshCounter counter;
  const auto ceiling = RefreshCounter::ceilingFor(2);

  countAll(counter, std::vector<double>(5000, 0.2), ceiling);

  const auto recovered = countAll(counter, std::vector<double>(20, 2.0), ceiling);
  const auto presentsToRecover =
      std::distance(recovered.begin(), std::find(recovered.begin(), recovered.end(), 2)) + 1;

  // How far the carry was allowed to go is exactly how long it takes to work back, so the bound on
  // one is the bound on the other
  EXPECT_LE(presentsToRecover, ceiling + 2) << "took " << presentsToRecover << " presents to count a gap correctly";
}

TEST(RefreshCounterTest, CeilingLeavesRoomForOneLatePresent) {
  EXPECT_EQ(RefreshCounter::ceilingFor(6), 7);
  EXPECT_EQ(RefreshCounter::ceilingFor(12), 13);
}

// Never below MIN_CEILING, so a frame that is due every refresh can still catch up after a hitch
TEST(RefreshCounterTest, CeilingNeverDropsBelowTheFloor) {
  EXPECT_EQ(RefreshCounter::ceilingFor(0), RefreshCounter::MIN_CEILING);
  EXPECT_EQ(RefreshCounter::ceilingFor(1), RefreshCounter::MIN_CEILING);
  EXPECT_EQ(RefreshCounter::ceilingFor(3), RefreshCounter::MIN_CEILING);
}

TEST(RefreshCounterTest, ReportsOneWhenThePeriodIsNotKnown) {
  RefreshCounter counter;

  EXPECT_EQ(counter.observe(PERIOD_NS * 5, 0, RefreshCounter::MIN_CEILING), 1);
  EXPECT_EQ(counter.observe(PERIOD_NS * 5, -1, RefreshCounter::MIN_CEILING), 1);
}

TEST(RefreshCounterTest, ResetForgetsWhatWasCarried) {
  const auto ceiling = RefreshCounter::ceilingFor(2);
  const auto gap = static_cast<int64_t>(1.5 * PERIOD_NS);

  RefreshCounter carried;
  EXPECT_EQ(carried.observe(gap, PERIOD_NS, ceiling), 1);
  EXPECT_EQ(carried.observe(gap, PERIOD_NS, ceiling), 2) << "half a refresh was carried into the second gap";

  RefreshCounter forgotten;
  EXPECT_EQ(forgotten.observe(gap, PERIOD_NS, ceiling), 1);
  forgotten.reset();

  EXPECT_EQ(forgotten.observe(gap, PERIOD_NS, ceiling), 1) << "reset did not forget the half refresh";
}

} // namespace firelight::emulation

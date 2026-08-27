// TODO: NEEDS REVIEW
#include <emulation/precision_waiter.hpp>
#include <gtest/gtest.h>

using firelight::emulation::PrecisionWaiter;

namespace {
constexpr int64_t MS = 1000000;

/**
 * Reports the same overshoot repeatedly, for settling behaviour
 */
void report(PrecisionWaiter &waiter, const int64_t overshootNs, const int times) {
  for (auto i = 0; i < times; ++i) {
    waiter.noteOvershoot(overshootNs);
  }
}
} // namespace

// TODO
// A host whose sleeps land on time should not be spinning for one that does not
TEST(PrecisionWaiterTest, StartsAtTheFloor) {
  const PrecisionWaiter waiter;
  EXPECT_EQ(waiter.getSpinMarginNs(), PrecisionWaiter::MIN_SPIN_MARGIN_NS);
}

// TODO
// The wait that overshot is already late, and so is every one after it until the margin covers them
TEST(PrecisionWaiterTest, WidensImmediatelyOnALateSleep) {
  PrecisionWaiter waiter;

  waiter.noteOvershoot(3 * MS);

  EXPECT_EQ(waiter.getSpinMarginNs(), 3 * MS + PrecisionWaiter::SPIN_MARGIN_HEADROOM_NS)
      << "one bad sleep should be covered by the next wait, not the hundredth";
}

// TODO
// Covering a sleep exactly leaves nothing for the next one that is a fraction worse
TEST(PrecisionWaiterTest, LeavesHeadroomAboveTheWorstSleepSeen) {
  PrecisionWaiter waiter;

  waiter.noteOvershoot(2 * MS);

  EXPECT_GT(waiter.getSpinMarginNs(), 2 * MS);
}

// TODO
// A host with a bad timer must not cost a whole core to hide it
TEST(PrecisionWaiterTest, NeverSpinsMoreThanTheCeiling) {
  PrecisionWaiter waiter;

  waiter.noteOvershoot(100 * MS);

  EXPECT_EQ(waiter.getSpinMarginNs(), PrecisionWaiter::MAX_SPIN_MARGIN_NS);
}

// TODO
// The sleeps that matter are the rare worst ones, so a margin that sags back between them stops
// covering the thing it was widened for
TEST(PrecisionWaiterTest, GivesBackWhatABadSleepBoughtOnlySlowly) {
  PrecisionWaiter waiter;
  waiter.noteOvershoot(4 * MS);
  const auto widened = waiter.getSpinMarginNs();

  report(waiter, 0, 60);

  EXPECT_GT(waiter.getSpinMarginNs(), widened / 2)
      << "a second of good sleeps should not undo what a bad one established";
  EXPECT_LT(waiter.getSpinMarginNs(), widened) << "and it should be coming down";
}

// TODO
// A host that is accurate for long enough should end up spinning as little as it is allowed to
TEST(PrecisionWaiterTest, SettlesToTheFloorOnAnAccurateHost) {
  PrecisionWaiter waiter;
  waiter.noteOvershoot(8 * MS);

  report(waiter, 0, 200000);

  EXPECT_EQ(waiter.getSpinMarginNs(), PrecisionWaiter::MIN_SPIN_MARGIN_NS);
}

// TODO
// A sleep inside the floor is not late; nothing about it should widen anything
TEST(PrecisionWaiterTest, ASleepInsideTheFloorChangesNothing) {
  PrecisionWaiter waiter;

  waiter.noteOvershoot(1000);

  EXPECT_EQ(waiter.getSpinMarginNs(), PrecisionWaiter::MIN_SPIN_MARGIN_NS);
}

// TODO
// A sleep that returned early is not a sleep that was late
TEST(PrecisionWaiterTest, AnEarlySleepDoesNotWiden) {
  PrecisionWaiter waiter;

  waiter.noteOvershoot(-2 * MS);

  EXPECT_EQ(waiter.getSpinMarginNs(), PrecisionWaiter::MIN_SPIN_MARGIN_NS);
}

// TODO
// Repeated late sleeps of the same size should settle, not climb toward the ceiling
TEST(PrecisionWaiterTest, HoldsSteadyAgainstAConsistentlyLateHost) {
  PrecisionWaiter waiter;

  report(waiter, 2 * MS, 1000);

  EXPECT_EQ(waiter.getSpinMarginNs(), 2 * MS + PrecisionWaiter::SPIN_MARGIN_HEADROOM_NS);
}

// TODO
// Nothing to wait for is not a wait, and asking the host for one would be a syscall for nothing
TEST(PrecisionWaiterTest, RefusesAWaitOfNoLength) {
  PrecisionWaiter waiter;

  EXPECT_FALSE(waiter.sleepFor(0));
  EXPECT_FALSE(waiter.sleepFor(-1));
}

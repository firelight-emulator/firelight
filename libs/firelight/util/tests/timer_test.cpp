#include <firelight/util/timer.hpp>

#include <gtest/gtest.h>

namespace firelight::util {

using namespace std::chrono;

/** A timer on a clock that only moves when the test advances m_now */
class TimerTest : public testing::Test {
protected:
  steady_clock::time_point m_now{hours(1)};
  Timer m_timer{[this] { return m_now; }};
};

TEST_F(TimerTest, IsZeroBeforeStart) {
  m_now += 100ms;

  EXPECT_EQ(m_timer.elapsed(), 0ms);
  EXPECT_EQ(m_timer.pause(), 0ms);
}

TEST_F(TimerTest, CountsWhileRunning) {
  m_timer.start();
  m_now += 100ms;
  EXPECT_EQ(m_timer.elapsed(), 100ms);

  m_now += 50ms;
  EXPECT_EQ(m_timer.elapsed(), 150ms);
}

TEST_F(TimerTest, ElapsedDoesNotChangeTheTotal) {
  m_timer.start();
  m_now += 100ms;
  EXPECT_EQ(m_timer.elapsed(), 100ms);
  EXPECT_EQ(m_timer.elapsed(), 100ms);

  m_now += 100ms;
  EXPECT_EQ(m_timer.elapsed(), 200ms);
  EXPECT_EQ(m_timer.pause(), 200ms);
}

TEST_F(TimerTest, PauseStopsCounting) {
  m_timer.start();
  m_now += 100ms;
  EXPECT_EQ(m_timer.pause(), 100ms);

  m_now += 500ms;
  EXPECT_EQ(m_timer.elapsed(), 100ms);
  EXPECT_EQ(m_timer.pause(), 100ms);
}

TEST_F(TimerTest, StartWhileRunningDoesNotReset) {
  m_timer.start();
  m_now += 100ms;
  m_timer.start();
  m_now += 50ms;

  EXPECT_EQ(m_timer.elapsed(), 150ms);
}

TEST_F(TimerTest, StartAfterPauseResumes) {
  m_timer.start();
  m_now += 100ms;
  m_timer.pause();
  m_now += 500ms;
  m_timer.start();
  m_now += 50ms;

  EXPECT_EQ(m_timer.elapsed(), 150ms);
}

TEST_F(TimerTest, RestartReturnsTheTotalAndStartsFromZero) {
  m_timer.start();
  m_now += 100ms;
  EXPECT_EQ(m_timer.restart(), 100ms);
  EXPECT_EQ(m_timer.elapsed(), 0ms);

  m_now += 30ms;
  EXPECT_EQ(m_timer.elapsed(), 30ms);
}

TEST_F(TimerTest, RestartIncludesTimeFromBeforeAPause) {
  m_timer.start();
  m_now += 100ms;
  m_timer.pause();
  m_now += 500ms;
  m_timer.start();
  m_now += 50ms;

  EXPECT_EQ(m_timer.restart(), 150ms);
}

TEST_F(TimerTest, RestartWhilePausedStartsRunning) {
  m_timer.start();
  m_now += 100ms;
  m_timer.pause();
  m_now += 500ms;
  EXPECT_EQ(m_timer.restart(), 100ms);

  m_now += 40ms;
  EXPECT_EQ(m_timer.elapsed(), 40ms);
}

TEST_F(TimerTest, RestartBeforeStartReturnsZeroAndStartsRunning) {
  m_now += 100ms;
  EXPECT_EQ(m_timer.restart(), 0ms);

  m_now += 40ms;
  EXPECT_EQ(m_timer.elapsed(), 40ms);
}

TEST_F(TimerTest, PartialMillisecondsAddUpAcrossPauses) {
  for (int i = 0; i < 100; ++i) {
    m_timer.start();
    m_now += 1900us;
    m_timer.pause();
  }

  EXPECT_EQ(m_timer.elapsed(), 190ms);
}

} // namespace firelight::util

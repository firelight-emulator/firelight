#include <firelight/activity/play_session.hpp>
#include <firelight/activity/sqlite_activity_log.hpp>

#include <gtest/gtest.h>

#include <string>

namespace firelight::activity {

namespace {
PlaySession makeSession(const std::string &hash, uint64_t start, uint64_t end,
                        uint64_t durationMillis = 0) {
  PlaySession s;
  s.contentHash = hash;
  s.slotNumber = 1;
  s.startTime = start;
  s.endTime = end;
  s.unpausedDurationMillis = durationMillis;
  return s;
}
} // namespace

// TODO
// NOTE: getDatabase() caches the :memory: connection per-thread and there is no
// teardown, so instances on this thread share one database. Each test therefore
// uses a unique content hash and only queries by that hash to stay isolated
class SqliteActivityLogTest : public testing::Test {
protected:
  SqliteActivityLog m_log{":memory:"};
};

TEST_F(SqliteActivityLogTest, CreateAssignsIdAndGetLatestReturnsIt) {
  auto session = makeSession("hash_create", 1000, 2000, 5000);
  ASSERT_TRUE(m_log.createPlaySession(session));
  EXPECT_GT(session.id, 0);

  const auto latest = m_log.getLatestPlaySession("hash_create");
  ASSERT_TRUE(latest.has_value());
  EXPECT_EQ(latest->startTime, 1000u);
  EXPECT_EQ(latest->endTime, 2000u);
  // Duration is persisted at second granularity, so whole seconds round-trip
  EXPECT_EQ(latest->unpausedDurationMillis, 5000u);
}

TEST_F(SqliteActivityLogTest, RejectsInvalidSessions) {
  auto noHash = makeSession("", 1000, 2000);
  EXPECT_FALSE(m_log.createPlaySession(noHash));

  auto noStart = makeSession("hash_reject", 0, 2000);
  EXPECT_FALSE(m_log.createPlaySession(noStart));

  auto noEnd = makeSession("hash_reject", 1000, 0);
  EXPECT_FALSE(m_log.createPlaySession(noEnd));

  EXPECT_FALSE(m_log.getLatestPlaySession("hash_reject").has_value());
}

TEST_F(SqliteActivityLogTest, GetPlaySessionsFiltersByContentHash) {
  auto a1 = makeSession("hash_filterA", 1000, 2000);
  auto a2 = makeSession("hash_filterA", 3000, 4000);
  auto b1 = makeSession("hash_filterB", 1500, 2500);
  ASSERT_TRUE(m_log.createPlaySession(a1));
  ASSERT_TRUE(m_log.createPlaySession(a2));
  ASSERT_TRUE(m_log.createPlaySession(b1));

  EXPECT_EQ(m_log.getPlaySessions("hash_filterA").size(), 2u);
  EXPECT_EQ(m_log.getPlaySessions("hash_filterB").size(), 1u);
}

TEST_F(SqliteActivityLogTest, GetLatestReturnsMostRecentByStartTime) {
  auto older = makeSession("hash_latest", 1000, 2000);
  auto newer = makeSession("hash_latest", 5000, 6000);
  ASSERT_TRUE(m_log.createPlaySession(older));
  ASSERT_TRUE(m_log.createPlaySession(newer));

  const auto latest = m_log.getLatestPlaySession("hash_latest");
  ASSERT_TRUE(latest.has_value());
  EXPECT_EQ(latest->startTime, 5000u);
}

TEST_F(SqliteActivityLogTest, GetLatestForUnknownHashIsEmpty) {
  EXPECT_FALSE(m_log.getLatestPlaySession("hash_never_written").has_value());
}

} // namespace firelight::activity

#include "db/database_inspector.hpp"

#include <firelight/activity/play_session.hpp>
#include <firelight/activity/sqlite_activity_log.hpp>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <gtest/gtest.h>
#include <string>

namespace firelight::activity {

namespace {
PlaySession makeSession(const std::string &hash, uint64_t start, uint64_t end, uint64_t durationMillis = 0) {
  PlaySession s;
  s.contentHash = hash;
  s.saveSlot = 1;
  s.startedAt = start;
  s.endedAt = end;
  s.unpausedDurationMillis = durationMillis;
  return s;
}
} // namespace

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
  EXPECT_EQ(latest->startedAt, 1000u);
  EXPECT_EQ(latest->endedAt, 2000u);
  // Duration is persisted in milliseconds, so nothing is rounded away
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
  EXPECT_EQ(latest->startedAt, 5000u);
}

TEST_F(SqliteActivityLogTest, GetLatestForUnknownHashIsEmpty) {
  EXPECT_FALSE(m_log.getLatestPlaySession("hash_never_written").has_value());
}

// The whole point of the migration adoption: a session written to a file DB
// survives being closed and reopened, and the store reports schema version 1
TEST(SqliteActivityLogFileTest, DataAndSchemaSurviveReopen) {
  namespace fs = std::filesystem;
  static std::atomic_int counter{0};
  const auto unique =
      std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + "_" + std::to_string(counter++);
  const auto dbPath = (fs::temp_directory_path() / ("fl_activity_test_" + unique + ".db")).string();

  {
    SqliteActivityLog log(dbPath);
    auto session = makeSession("hash_persist", 1000, 2000, 7000);
    ASSERT_TRUE(log.createPlaySession(session));
  }

  {
    SqliteActivityLog reopened(dbPath);
    const auto latest = reopened.getLatestPlaySession("hash_persist");
    ASSERT_TRUE(latest.has_value());
    EXPECT_EQ(latest->startedAt, 1000u);
    EXPECT_EQ(latest->unpausedDurationMillis, 7000u);
  }

  const auto info = db::inspect(dbPath);
  EXPECT_TRUE(info.integrityOk);
  EXPECT_EQ(info.userVersion, 1);

  std::error_code ec;
  fs::remove(dbPath, ec);
}

// Durations used to be stored in seconds while the field was milliseconds, so every session
// lost its remainder going in and got it back as zeros
TEST_F(SqliteActivityLogTest, SubSecondDurationSurvives) {
  activity::PlaySession session;
  session.contentHash = "hash_precision";
  session.saveSlot = 1;
  session.startedAt = 1000;
  session.endedAt = 9500;
  session.unpausedDurationMillis = 8456;
  ASSERT_TRUE(m_log.createPlaySession(session));

  const auto latest = m_log.getLatestPlaySession("hash_precision");
  ASSERT_TRUE(latest.has_value());
  EXPECT_EQ(latest->unpausedDurationMillis, 8456u);
}

// Two entries turning out to be one game must not split somebody's playtime in half
TEST_F(SqliteActivityLogTest, TransferMovesSessionsToTheSurvivingHash) {
  auto first = makeSession("discTwoHash", 1000, 2000, 5000);
  auto second = makeSession("discTwoHash", 3000, 4000, 7000);
  auto other = makeSession("unrelatedHash", 5000, 6000, 9000);
  ASSERT_TRUE(m_log.createPlaySession(first));
  ASSERT_TRUE(m_log.createPlaySession(second));
  ASSERT_TRUE(m_log.createPlaySession(other));

  EXPECT_TRUE(m_log.transferSessions("discTwoHash", "discOneHash"));

  EXPECT_EQ(m_log.getPlaySessions("discOneHash").size(), 2u);
  EXPECT_TRUE(m_log.getPlaySessions("discTwoHash").empty());
  EXPECT_EQ(m_log.getPlaySessions("unrelatedHash").size(), 1u);
}

// Playtime already on the surviving hash is added to rather than replaced
TEST_F(SqliteActivityLogTest, TransferKeepsWhatTheSurvivorAlreadyPlayed) {
  auto onSurvivor = makeSession("discOneHash", 1000, 2000, 5000);
  auto onAbsorbed = makeSession("discTwoHash", 3000, 4000, 7000);
  ASSERT_TRUE(m_log.createPlaySession(onSurvivor));
  ASSERT_TRUE(m_log.createPlaySession(onAbsorbed));

  ASSERT_TRUE(m_log.transferSessions("discTwoHash", "discOneHash"));

  const auto sessions = m_log.getPlaySessions("discOneHash");
  ASSERT_EQ(sessions.size(), 2u);

  uint64_t total = 0;
  for (const auto &session : sessions) {
    total += session.unpausedDurationMillis;
  }
  EXPECT_EQ(total, 12000u);
}

TEST_F(SqliteActivityLogTest, TransferOfNothingIsHarmless) {
  EXPECT_FALSE(m_log.transferSessions("neverSeen", "alsoNeverSeen"));
  EXPECT_FALSE(m_log.transferSessions("sameHash", "sameHash"));
  EXPECT_FALSE(m_log.transferSessions("", "somewhere"));
}

} // namespace firelight::activity

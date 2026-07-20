#include "fake_save_database.hpp"

#include <firelight/saves/save_manager_impl.hpp>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

namespace firelight::saves {
namespace fs = std::filesystem;

namespace {
std::atomic_int g_counter{0};

std::vector<char> bytesOf(const std::string &s) { return {s.begin(), s.end()}; }

std::vector<uint8_t> uBytesOf(const std::string &s) { return {s.begin(), s.end()}; }

SuspendPoint makeSuspendPoint(const std::string &contentHash, const int saveSlotNumber, const std::string &state) {
  SuspendPoint point;
  point.contentHash = contentHash;
  point.saveSlotNumber = saveSlotNumber;
  point.state = uBytesOf(state);
  point.timestamp = 0;
  return point;
}
} // namespace

class SaveManagerTest : public testing::Test {
protected:
  fs::path m_tempDir;
  FakeSaveDatabase m_db;
  std::unique_ptr<SaveManager> m_saveManager;
  const std::string m_hash = "abc123hash";

  void SetUp() override {
    const auto unique =
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + "_" + std::to_string(g_counter++);
    m_tempDir = fs::temp_directory_path() / ("fl_saves_test_" + unique);
    fs::create_directories(m_tempDir);
    m_saveManager = std::make_unique<SaveManager>(m_tempDir.string(), m_db);
  }

  void TearDown() override {
    std::error_code ec;
    fs::remove_all(m_tempDir, ec);
  }
};

TEST_F(SaveManagerTest, EmptySlotsReportNoData) {
  const auto list = m_saveManager->getSaveFileInfoList(m_hash);
  ASSERT_EQ(list.size(), 8u);
  for (const auto &info : list) {
    EXPECT_FALSE(info.hasData);
  }
}

TEST_F(SaveManagerTest, WriteThenReadRoundTrips) {
  const auto data = bytesOf("hello-save-data");
  ASSERT_TRUE(m_saveManager->writeSaveData(m_hash, 1, Savefile(data)).get());

  const auto readBack = m_saveManager->readSaveData(m_hash, 1);
  ASSERT_TRUE(readBack.has_value());
  EXPECT_EQ(readBack->getSaveRamData(), data);
}

TEST_F(SaveManagerTest, WriteCreatesFileAtSlotPath) {
  ASSERT_TRUE(m_saveManager->writeSaveData(m_hash, 3, Savefile(bytesOf("x"))).get());
  const auto expected = m_tempDir / m_hash / "slot3" / "savefile.srm";
  EXPECT_TRUE(fs::exists(expected));
}

TEST_F(SaveManagerTest, FirstWriteCreatesMetadataSecondUpdates) {
  ASSERT_TRUE(m_saveManager->writeSaveData(m_hash, 1, Savefile(bytesOf("aaaa"))).get());
  EXPECT_EQ(m_db.createCount, 1);
  EXPECT_EQ(m_db.updateCount, 0);

  // Different data -> different MD5 -> an update, not a second create
  ASSERT_TRUE(m_saveManager->writeSaveData(m_hash, 1, Savefile(bytesOf("bbbb"))).get());
  EXPECT_EQ(m_db.createCount, 1);
  EXPECT_EQ(m_db.updateCount, 1);
}

TEST_F(SaveManagerTest, UnchangedDataSkipsRewrite) {
  const auto data = bytesOf("same-bytes");
  ASSERT_TRUE(m_saveManager->writeSaveData(m_hash, 1, Savefile(data)).get());
  EXPECT_EQ(m_db.createCount, 1);

  // Identical bytes: the MD5 matches, so the write short-circuits with no
  // further metadata churn
  ASSERT_TRUE(m_saveManager->writeSaveData(m_hash, 1, Savefile(data)).get());
  EXPECT_EQ(m_db.createCount, 1);
  EXPECT_EQ(m_db.updateCount, 0);
}

TEST_F(SaveManagerTest, SetsLastModifiedTimestamp) {
  ASSERT_TRUE(m_saveManager->writeSaveData(m_hash, 1, Savefile(bytesOf("data"))).get());
  const auto md = m_db.getSavefileMetadata(m_hash, 1);
  ASSERT_TRUE(md.has_value());
  EXPECT_GT(md->lastModifiedAt, 0);
}

//****************
// suspend points
//****************

TEST_F(SaveManagerTest, SuspendPointStateRoundTrips) {
  auto point = makeSuspendPoint(m_hash, 1, "suspend-state-bytes");
  m_saveManager->writeSuspendPoint(m_hash, 1, 0, point);

  const auto readBack = m_saveManager->readSuspendPoint(m_hash, 1, 0);
  ASSERT_TRUE(readBack.has_value());
  EXPECT_EQ(readBack->state, point.state);
}

TEST_F(SaveManagerTest, SuspendPointRoundTripsRcheevosStateAndImage) {
  auto point = makeSuspendPoint(m_hash, 1, "state");
  point.retroachievementsState = uBytesOf("rcheevos-bytes");
  point.image.pngData = uBytesOf("fake-png-bytes");
  m_saveManager->writeSuspendPoint(m_hash, 1, 0, point);

  const auto readBack = m_saveManager->readSuspendPoint(m_hash, 1, 0);
  ASSERT_TRUE(readBack.has_value());
  EXPECT_EQ(readBack->retroachievementsState, point.retroachievementsState);
  EXPECT_EQ(readBack->image.pngData, point.image.pngData);
}

TEST_F(SaveManagerTest, SuspendPointOmitsOptionalFilesWhenEmpty) {
  const auto point = makeSuspendPoint(m_hash, 1, "state-only");
  m_saveManager->writeSuspendPoint(m_hash, 1, 0, point);

  const auto readBack = m_saveManager->readSuspendPoint(m_hash, 1, 0);
  ASSERT_TRUE(readBack.has_value());
  EXPECT_TRUE(readBack->retroachievementsState.empty());
  EXPECT_TRUE(readBack->image.pngData.empty());
}

// index is zero-based but lands on a one-based slot directory, so the three
// disk functions have to agree on the conversion
TEST_F(SaveManagerTest, SuspendPointIndexMapsToOneBasedSlotDirectory) {
  const auto point = makeSuspendPoint(m_hash, 2, "state");
  m_saveManager->writeSuspendPoint(m_hash, 2, 0, point);

  const auto expected = m_tempDir / m_hash / "slot2" / "suspendpoints" / "slot1" / "suspendpoint.state";
  EXPECT_TRUE(fs::exists(expected));
}

TEST_F(SaveManagerTest, SuspendPointsAtDifferentIndicesDoNotCollide) {
  m_saveManager->writeSuspendPoint(m_hash, 1, 0, makeSuspendPoint(m_hash, 1, "first"));
  m_saveManager->writeSuspendPoint(m_hash, 1, 1, makeSuspendPoint(m_hash, 1, "second"));

  const auto first = m_saveManager->readSuspendPoint(m_hash, 1, 0);
  const auto second = m_saveManager->readSuspendPoint(m_hash, 1, 1);
  ASSERT_TRUE(first.has_value());
  ASSERT_TRUE(second.has_value());
  EXPECT_EQ(first->state, uBytesOf("first"));
  EXPECT_EQ(second->state, uBytesOf("second"));
}

TEST_F(SaveManagerTest, SuspendPointsInDifferentSaveSlotsDoNotCollide) {
  m_saveManager->writeSuspendPoint(m_hash, 1, 0, makeSuspendPoint(m_hash, 1, "slot-one"));
  m_saveManager->writeSuspendPoint(m_hash, 2, 0, makeSuspendPoint(m_hash, 2, "slot-two"));

  const auto first = m_saveManager->readSuspendPoint(m_hash, 1, 0);
  const auto second = m_saveManager->readSuspendPoint(m_hash, 2, 0);
  ASSERT_TRUE(first.has_value());
  ASSERT_TRUE(second.has_value());
  EXPECT_EQ(first->state, uBytesOf("slot-one"));
  EXPECT_EQ(second->state, uBytesOf("slot-two"));
}

TEST_F(SaveManagerTest, LockedSuspendPointIsNotWritten) {
  auto point = makeSuspendPoint(m_hash, 1, "should-not-persist");
  point.locked = true;
  m_saveManager->writeSuspendPoint(m_hash, 1, 0, point);

  EXPECT_FALSE(m_saveManager->readSuspendPoint(m_hash, 1, 0).has_value());
}

TEST_F(SaveManagerTest, ReadingMissingSuspendPointReturnsNullopt) {
  EXPECT_FALSE(m_saveManager->readSuspendPoint(m_hash, 1, 0).has_value());
}

TEST_F(SaveManagerTest, DeletedSuspendPointNoLongerReads) {
  m_saveManager->writeSuspendPoint(m_hash, 1, 0, makeSuspendPoint(m_hash, 1, "state"));
  ASSERT_TRUE(m_saveManager->readSuspendPoint(m_hash, 1, 0).has_value());

  m_saveManager->deleteSuspendPoint(m_hash, 1, 0);
  EXPECT_FALSE(m_saveManager->readSuspendPoint(m_hash, 1, 0).has_value());
}

TEST_F(SaveManagerTest, DeletingOneSuspendPointLeavesOthers) {
  m_saveManager->writeSuspendPoint(m_hash, 1, 0, makeSuspendPoint(m_hash, 1, "keep"));
  m_saveManager->writeSuspendPoint(m_hash, 1, 1, makeSuspendPoint(m_hash, 1, "drop"));

  m_saveManager->deleteSuspendPoint(m_hash, 1, 1);

  EXPECT_TRUE(m_saveManager->readSuspendPoint(m_hash, 1, 0).has_value());
  EXPECT_FALSE(m_saveManager->readSuspendPoint(m_hash, 1, 1).has_value());
}

TEST_F(SaveManagerTest, SuspendPointCarriesLockedFlagFromMetadata) {
  m_saveManager->writeSuspendPoint(m_hash, 1, 0, makeSuspendPoint(m_hash, 1, "state"));

  auto metadata = m_db.getSuspendPointMetadata(m_hash, 1, 1);
  ASSERT_TRUE(metadata.has_value());
  metadata->locked = true;
  m_db.updateSuspendPointMetadata(*metadata);

  const auto readBack = m_saveManager->readSuspendPoint(m_hash, 1, 0);
  ASSERT_TRUE(readBack.has_value());
  EXPECT_TRUE(readBack->locked);
}

TEST_F(SaveManagerTest, SuspendPointTimestampComesFromTheStateFile) {
  m_saveManager->writeSuspendPoint(m_hash, 1, 0, makeSuspendPoint(m_hash, 1, "state"));

  const auto readBack = m_saveManager->readSuspendPoint(m_hash, 1, 0);
  ASSERT_TRUE(readBack.has_value());
  EXPECT_GT(readBack->timestamp, 0);
}

// The saveSlotNumber parameter is authoritative; a disagreeing struct field must
// not send the file to a different slot than the one the caller and the event use
TEST_F(SaveManagerTest, SuspendPointUsesParameterSlotNotStructField) {
  auto point = makeSuspendPoint(m_hash, 5, "state");
  point.saveSlotNumber = 9; // deliberately disagrees with the parameter
  m_saveManager->writeSuspendPoint(m_hash, 3, 0, point);

  const auto atParameterSlot = m_tempDir / m_hash / "slot3" / "suspendpoints" / "slot1" / "suspendpoint.state";
  const auto atStructSlot = m_tempDir / m_hash / "slot9" / "suspendpoints" / "slot1" / "suspendpoint.state";
  EXPECT_TRUE(fs::exists(atParameterSlot));
  EXPECT_FALSE(fs::exists(atStructSlot));

  EXPECT_TRUE(m_saveManager->readSuspendPoint(m_hash, 3, 0).has_value());
}

TEST_F(SaveManagerTest, RewritingSuspendPointReplacesState) {
  m_saveManager->writeSuspendPoint(m_hash, 1, 0, makeSuspendPoint(m_hash, 1, "original"));
  m_saveManager->writeSuspendPoint(m_hash, 1, 0, makeSuspendPoint(m_hash, 1, "replacement"));

  const auto readBack = m_saveManager->readSuspendPoint(m_hash, 1, 0);
  ASSERT_TRUE(readBack.has_value());
  EXPECT_EQ(readBack->state, uBytesOf("replacement"));
}

} // namespace firelight::saves

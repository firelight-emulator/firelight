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

} // namespace firelight::saves

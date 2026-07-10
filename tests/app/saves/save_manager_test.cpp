#include "fake_userdata_database.hpp"

#include <firelight/saves/save_manager_impl.hpp>

#include <gtest/gtest.h>

#include <QFileInfo>
#include <QTemporaryDir>

#include <string>
#include <vector>

namespace firelight::saves {

namespace {
std::vector<char> bytesOf(const std::string &s) {
  return {s.begin(), s.end()};
}
} // namespace

class SaveManagerTest : public testing::Test {
protected:
  QTemporaryDir m_tempDir;
  FakeUserdataDatabase m_db;
  std::unique_ptr<SaveManager> m_saveManager;
  const std::string m_hash = "abc123hash";

  void SetUp() override {
    ASSERT_TRUE(m_tempDir.isValid());
    m_saveManager =
        std::make_unique<SaveManager>(m_tempDir.path().toStdString(), m_db);
    // Force the temp dir regardless of any persisted QSettings value.
    m_saveManager->setSaveDirectory(m_tempDir.path().toStdString());
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
  ASSERT_TRUE(
      m_saveManager->writeSaveData(m_hash, 3, Savefile(bytesOf("x"))).get());
  const QString expected = m_tempDir.path() + "/" +
                           QString::fromStdString(m_hash) +
                           "/slot3/savefile.srm";
  EXPECT_TRUE(QFileInfo::exists(expected));
}

TEST_F(SaveManagerTest, FirstWriteCreatesMetadataSecondUpdates) {
  ASSERT_TRUE(
      m_saveManager->writeSaveData(m_hash, 1, Savefile(bytesOf("aaaa"))).get());
  EXPECT_EQ(m_db.createCount, 1);
  EXPECT_EQ(m_db.updateCount, 0);

  // Different data -> different MD5 -> an update, not a second create.
  ASSERT_TRUE(
      m_saveManager->writeSaveData(m_hash, 1, Savefile(bytesOf("bbbb"))).get());
  EXPECT_EQ(m_db.createCount, 1);
  EXPECT_EQ(m_db.updateCount, 1);
}

TEST_F(SaveManagerTest, UnchangedDataSkipsRewrite) {
  const auto data = bytesOf("same-bytes");
  ASSERT_TRUE(m_saveManager->writeSaveData(m_hash, 1, Savefile(data)).get());
  EXPECT_EQ(m_db.createCount, 1);

  // Identical bytes: the MD5 matches, so the write short-circuits with no
  // further metadata churn.
  ASSERT_TRUE(m_saveManager->writeSaveData(m_hash, 1, Savefile(data)).get());
  EXPECT_EQ(m_db.createCount, 1);
  EXPECT_EQ(m_db.updateCount, 0);
}

TEST_F(SaveManagerTest, SetsLastModifiedTimestamp) {
  ASSERT_TRUE(
      m_saveManager->writeSaveData(m_hash, 1, Savefile(bytesOf("data"))).get());
  const auto md = m_db.getSavefileMetadata(m_hash, 1);
  ASSERT_TRUE(md.has_value());
  EXPECT_GT(md->lastModifiedAt, 0u);
}

// --- copySavefilesForward (save-compatible mod-update migration) ---

TEST_F(SaveManagerTest, CopyForwardMovesSramToNewHash) {
  const std::string oldHash = "mod-v1";
  const std::string newHash = "mod-v2";
  const auto data = bytesOf("progress");
  ASSERT_TRUE(m_saveManager->writeSaveData(oldHash, 1, Savefile(data)).get());

  const int copied = m_saveManager->copySavefilesForward(oldHash, newHash);
  EXPECT_EQ(copied, 1);

  // The new hash now has the same SRAM...
  const auto readBack = m_saveManager->readSaveData(newHash, 1);
  ASSERT_TRUE(readBack.has_value());
  EXPECT_EQ(readBack->getSaveRamData(), data);
  // ...and it is indexed so it shows without a re-scan.
  EXPECT_TRUE(m_db.getSavefileMetadata(newHash, 1).has_value());
  // The original save is left intact under the old hash.
  const auto original = m_saveManager->readSaveData(oldHash, 1);
  ASSERT_TRUE(original.has_value());
  EXPECT_EQ(original->getSaveRamData(), data);
}

TEST_F(SaveManagerTest, CopyForwardCopiesEveryPopulatedSlot) {
  const std::string oldHash = "mod-v1";
  const std::string newHash = "mod-v2";
  ASSERT_TRUE(
      m_saveManager->writeSaveData(oldHash, 1, Savefile(bytesOf("a"))).get());
  ASSERT_TRUE(
      m_saveManager->writeSaveData(oldHash, 4, Savefile(bytesOf("b"))).get());

  EXPECT_EQ(m_saveManager->copySavefilesForward(oldHash, newHash), 2);
  EXPECT_TRUE(m_saveManager->readSaveData(newHash, 1).has_value());
  EXPECT_TRUE(m_saveManager->readSaveData(newHash, 4).has_value());
  EXPECT_FALSE(m_saveManager->readSaveData(newHash, 2).has_value());
}

TEST_F(SaveManagerTest, CopyForwardNeverOverwritesExistingDestinationSave) {
  const std::string oldHash = "mod-v1";
  const std::string newHash = "mod-v2";
  ASSERT_TRUE(
      m_saveManager->writeSaveData(oldHash, 1, Savefile(bytesOf("old"))).get());
  const auto keep = bytesOf("already-here");
  ASSERT_TRUE(m_saveManager->writeSaveData(newHash, 1, Savefile(keep)).get());

  // Destination slot already has a save, so nothing is copied over it.
  EXPECT_EQ(m_saveManager->copySavefilesForward(oldHash, newHash), 0);
  const auto readBack = m_saveManager->readSaveData(newHash, 1);
  ASSERT_TRUE(readBack.has_value());
  EXPECT_EQ(readBack->getSaveRamData(), keep);
}

TEST_F(SaveManagerTest, CopyForwardDoesNotMigrateSuspendPoints) {
  const std::string oldHash = "mod-v1";
  const std::string newHash = "mod-v2";
  ASSERT_TRUE(
      m_saveManager->writeSaveData(oldHash, 1, Savefile(bytesOf("x"))).get());
  SuspendPoint sp;
  sp.state = {1, 2, 3, 4};
  sp.saveSlotNumber = 1;
  m_saveManager->writeSuspendPoint(oldHash, 1, 0, sp);
  ASSERT_TRUE(m_saveManager->readSuspendPoint(oldHash, 1, 0).has_value());

  m_saveManager->copySavefilesForward(oldHash, newHash);

  // SRAM migrated, but suspend points are deliberately left behind.
  EXPECT_TRUE(m_saveManager->readSaveData(newHash, 1).has_value());
  EXPECT_FALSE(m_saveManager->readSuspendPoint(newHash, 1, 0).has_value());
}

TEST_F(SaveManagerTest, CopyForwardIsNoOpForSameOrEmptyHash) {
  ASSERT_TRUE(
      m_saveManager->writeSaveData("mod-v1", 1, Savefile(bytesOf("x"))).get());
  EXPECT_EQ(m_saveManager->copySavefilesForward("mod-v1", "mod-v1"), 0);
  EXPECT_EQ(m_saveManager->copySavefilesForward("", "mod-v2"), 0);
  EXPECT_EQ(m_saveManager->copySavefilesForward("mod-v1", ""), 0);
}

TEST_F(SaveManagerTest, CopyForwardWithNoSourceSaveCopiesNothing) {
  EXPECT_EQ(m_saveManager->copySavefilesForward("absent", "mod-v2"), 0);
  EXPECT_FALSE(m_saveManager->readSaveData("mod-v2", 1).has_value());
}

} // namespace firelight::saves

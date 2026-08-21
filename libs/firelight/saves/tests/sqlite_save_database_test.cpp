#include <firelight/saves/sqlite_save_database.hpp>

#include <gtest/gtest.h>

namespace firelight::saves {

class SqliteSaveDatabaseTest : public testing::Test {
protected:
  SqliteSaveDatabase db{":memory:"};
};

TEST_F(SqliteSaveDatabaseTest, CreateSavefileSetsId) {
  SavefileMetadata m;
  m.contentHash = "hash";
  m.saveSlot = 1;
  m.savefileMd5 = "abc";
  ASSERT_TRUE(db.createSavefileMetadata(m));
  EXPECT_GT(m.id, 0);
}

TEST_F(SqliteSaveDatabaseTest, GetSavefileRoundTrips) {
  SavefileMetadata m;
  m.contentHash = "hash";
  m.saveSlot = 2;
  m.savefileMd5 = "md5v1";
  m.lastModifiedAt = 111;
  ASSERT_TRUE(db.createSavefileMetadata(m));

  const auto got = db.getSavefileMetadata("hash", 2);
  ASSERT_TRUE(got.has_value());
  EXPECT_EQ(got->contentHash, "hash");
  EXPECT_EQ(got->saveSlot, 2u);
  EXPECT_EQ(got->savefileMd5, "md5v1");
  EXPECT_EQ(got->lastModifiedAt, 111);
}

TEST_F(SqliteSaveDatabaseTest, UpdateSavefile) {
  SavefileMetadata m;
  m.contentHash = "hash";
  m.saveSlot = 1;
  m.savefileMd5 = "old";
  ASSERT_TRUE(db.createSavefileMetadata(m));

  m.savefileMd5 = "new";
  m.lastModifiedAt = 999;
  ASSERT_TRUE(db.updateSavefileMetadata(m));

  const auto got = db.getSavefileMetadata("hash", 1);
  ASSERT_TRUE(got.has_value());
  EXPECT_EQ(got->savefileMd5, "new");
  EXPECT_EQ(got->lastModifiedAt, 999);
}

TEST_F(SqliteSaveDatabaseTest, GetSavefilesForContent) {
  for (int slot = 1; slot <= 3; ++slot) {
    SavefileMetadata m;
    m.contentHash = "hash";
    m.saveSlot = slot;
    m.savefileMd5 = "x";
    ASSERT_TRUE(db.createSavefileMetadata(m));
  }
  EXPECT_EQ(db.getSavefileMetadataForContent("hash").size(), 3u);
}

TEST_F(SqliteSaveDatabaseTest, SuspendPointCrudRoundTrips) {
  SuspendPointMetadata m;
  m.contentHash = "hash";
  m.saveSlot = 1;
  m.pointIndex = 1;
  m.locked = false;
  ASSERT_TRUE(db.createSuspendPointMetadata(m));
  EXPECT_GT(m.id, 0);

  auto got = db.getSuspendPointMetadata("hash", 1, 1);
  ASSERT_TRUE(got.has_value());
  EXPECT_FALSE(got->locked);

  // updateSuspendPointMetadata persists the lock flag
  got->locked = true;
  ASSERT_TRUE(db.updateSuspendPointMetadata(*got));
  EXPECT_TRUE(db.getSuspendPointMetadata("hash", 1, 1)->locked);
}

TEST_F(SqliteSaveDatabaseTest, GetSuspendPointsForContentReturnsRows) {
  // Regression: the old implementation never populated the result vector and
  // always returned empty
  for (int slot = 1; slot <= 2; ++slot) {
    SuspendPointMetadata m;
    m.contentHash = "hash";
    m.saveSlot = 1;
    m.pointIndex = slot;
    ASSERT_TRUE(db.createSuspendPointMetadata(m));
  }
  EXPECT_EQ(db.getSuspendPointMetadataForContent("hash", 1).size(), 2u);
}

TEST_F(SqliteSaveDatabaseTest, DeleteSuspendPointActuallyDeletes) {
  // Regression: the old implementation was a no-op that leaked rows
  SuspendPointMetadata m;
  m.contentHash = "hash";
  m.saveSlot = 1;
  m.pointIndex = 1;
  ASSERT_TRUE(db.createSuspendPointMetadata(m));

  ASSERT_TRUE(db.deleteSuspendPointMetadata(m.id));
  EXPECT_FALSE(db.getSuspendPointMetadata("hash", 1, 1).has_value());
}

// Two playthroughs of one game each keep their own suspend points, so index 0 of save slot 1
// and index 0 of save slot 2 are different rows. The table was keyed without the save slot,
// which made the second one fail to insert
TEST_F(SqliteSaveDatabaseTest, SuspendPointsAreScopedToTheirSaveSlot) {
  SuspendPointMetadata inSlotOne;
  inSlotOne.contentHash = "hash";
  inSlotOne.saveSlot = 1;
  inSlotOne.pointIndex = 0;
  ASSERT_TRUE(db.createSuspendPointMetadata(inSlotOne));

  SuspendPointMetadata inSlotTwo;
  inSlotTwo.contentHash = "hash";
  inSlotTwo.saveSlot = 2;
  inSlotTwo.pointIndex = 0;
  ASSERT_TRUE(db.createSuspendPointMetadata(inSlotTwo));

  const auto first = db.getSuspendPointMetadata("hash", 1, 0);
  const auto second = db.getSuspendPointMetadata("hash", 2, 0);
  ASSERT_TRUE(first.has_value());
  ASSERT_TRUE(second.has_value());
  EXPECT_EQ(first->saveSlot, 1);
  EXPECT_EQ(second->saveSlot, 2);
}

// The same point in the same slot is still one row
TEST_F(SqliteSaveDatabaseTest, TheSameSuspendPointTwiceIsOneRow) {
  SuspendPointMetadata point;
  point.contentHash = "hash";
  point.saveSlot = 1;
  point.pointIndex = 0;
  ASSERT_TRUE(db.createSuspendPointMetadata(point));

  SuspendPointMetadata again = point;
  again.id = -1;
  db.createSuspendPointMetadata(again);

  EXPECT_EQ(db.getSuspendPointMetadataForContent("hash", 1).size(), 1u);
}

} // namespace firelight::saves

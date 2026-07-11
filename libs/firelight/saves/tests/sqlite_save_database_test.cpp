#include <firelight/saves/sqlite_save_database.hpp>

#include <gtest/gtest.h>

namespace firelight::saves {

class SqliteSaveDatabaseTest : public testing::Test {
protected:
  SqliteSaveDatabase db{":memory:"};
};

TEST_F(SqliteSaveDatabaseTest, CreateSavefileSetsId) {
  SavefileMetadata m;
  m.contentId = "hash";
  m.slotNumber = 1;
  m.savefileMd5 = "abc";
  ASSERT_TRUE(db.createSavefileMetadata(m));
  EXPECT_GT(m.id, 0);
}

TEST_F(SqliteSaveDatabaseTest, GetSavefileRoundTrips) {
  SavefileMetadata m;
  m.contentId = "hash";
  m.slotNumber = 2;
  m.savefileMd5 = "md5v1";
  m.lastModifiedAt = 111;
  ASSERT_TRUE(db.createSavefileMetadata(m));

  const auto got = db.getSavefileMetadata("hash", 2);
  ASSERT_TRUE(got.has_value());
  EXPECT_EQ(got->contentId, "hash");
  EXPECT_EQ(got->slotNumber, 2u);
  EXPECT_EQ(got->savefileMd5, "md5v1");
  EXPECT_EQ(got->lastModifiedAt, 111);
}

TEST_F(SqliteSaveDatabaseTest, UpdateSavefile) {
  SavefileMetadata m;
  m.contentId = "hash";
  m.slotNumber = 1;
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
    m.contentId = "hash";
    m.slotNumber = slot;
    m.savefileMd5 = "x";
    ASSERT_TRUE(db.createSavefileMetadata(m));
  }
  EXPECT_EQ(db.getSavefileMetadataForContent("hash").size(), 3u);
}

TEST_F(SqliteSaveDatabaseTest, SuspendPointCrudRoundTrips) {
  SuspendPointMetadata m;
  m.contentId = "hash";
  m.saveSlotNumber = 1;
  m.slotNumber = 1;
  m.locked = false;
  ASSERT_TRUE(db.createSuspendPointMetadata(m));
  EXPECT_GT(m.id, 0);

  auto got = db.getSuspendPointMetadata("hash", 1, 1);
  ASSERT_TRUE(got.has_value());
  EXPECT_FALSE(got->locked);

  // updateSuspendPointMetadata persists the lock flag.
  got->locked = true;
  ASSERT_TRUE(db.updateSuspendPointMetadata(*got));
  EXPECT_TRUE(db.getSuspendPointMetadata("hash", 1, 1)->locked);
}

TEST_F(SqliteSaveDatabaseTest, GetSuspendPointsForContentReturnsRows) {
  // Regression: the old implementation never populated the result vector and
  // always returned empty.
  for (int slot = 1; slot <= 2; ++slot) {
    SuspendPointMetadata m;
    m.contentId = "hash";
    m.saveSlotNumber = 1;
    m.slotNumber = slot;
    ASSERT_TRUE(db.createSuspendPointMetadata(m));
  }
  EXPECT_EQ(db.getSuspendPointMetadataForContent("hash", 1).size(), 2u);
}

TEST_F(SqliteSaveDatabaseTest, DeleteSuspendPointActuallyDeletes) {
  // Regression: the old implementation was a no-op that leaked rows.
  SuspendPointMetadata m;
  m.contentId = "hash";
  m.saveSlotNumber = 1;
  m.slotNumber = 1;
  ASSERT_TRUE(db.createSuspendPointMetadata(m));

  ASSERT_TRUE(db.deleteSuspendPointMetadata(m.id));
  EXPECT_FALSE(db.getSuspendPointMetadata("hash", 1, 1).has_value());
}

} // namespace firelight::saves

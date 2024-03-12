#include "../../../src/app/db/sqlite_userdata_database.hpp"
#include <gtest/gtest.h>

namespace firelight::db {

class SqliteUserdataDatabaseTest : public testing::Test {
protected:
  std::filesystem::path temp_file_path;

  void SetUp() override {
    temp_file_path =
        std::filesystem::temp_directory_path() / "firelightuserdata.db";
  }

  void TearDown() override { std::filesystem::remove(temp_file_path); }
};

TEST_F(SqliteUserdataDatabaseTest, ConstructorTest) {
  SqliteUserdataDatabase db(temp_file_path.string());

  // Add assertions to check the initial state of the database
  ASSERT_TRUE(db.tableExists("savefile_metadata"));
  ASSERT_TRUE(db.tableExists("play_sessions"));
}

TEST_F(SqliteUserdataDatabaseTest, CreateSavefileMetadataSetsId) {
  SqliteUserdataDatabase db(temp_file_path.string());
  SavefileMetadata metadata;
  metadata.id = -1;
  metadata.contentMd5 = "1234567890";
  metadata.slotNumber = 1;
  metadata.savefileMd5 = "0987654321";
  metadata.lastModifiedAt = 1000;

  ASSERT_TRUE(db.createSavefileMetadata(metadata));
  ASSERT_NE(metadata.id, -1);

  const auto playlists = db.getSavefileMetadataForContent(metadata.contentMd5);
  ASSERT_EQ(playlists.size(), 1);
  ASSERT_EQ(playlists[0].id, metadata.id);
  ASSERT_EQ(playlists[0].contentMd5, metadata.contentMd5);
  ASSERT_EQ(playlists[0].slotNumber, metadata.slotNumber);
  ASSERT_EQ(playlists[0].savefileMd5, metadata.savefileMd5);
  ASSERT_EQ(playlists[0].lastModifiedAt, metadata.lastModifiedAt);
  ASSERT_TRUE(playlists[0].createdAt > 0);
}

TEST_F(SqliteUserdataDatabaseTest, GetSavefileMetadataTest) {
  SqliteUserdataDatabase db(temp_file_path.string());

  const auto result = db.getSavefileMetadata("1234567890", 1);
  ASSERT_FALSE(result.has_value());

  SavefileMetadata metadata;
  metadata.id = -1;
  metadata.contentMd5 = "1234567890";
  metadata.slotNumber = 1;
  metadata.savefileMd5 = "0987654321";
  metadata.lastModifiedAt = 1000;

  ASSERT_TRUE(db.createSavefileMetadata(metadata));
  ASSERT_NE(metadata.id, -1);

  const auto retrievedMetadata =
      db.getSavefileMetadata(metadata.contentMd5, metadata.slotNumber);
  ASSERT_TRUE(retrievedMetadata.has_value());
  ASSERT_EQ(retrievedMetadata->id, metadata.id);
  ASSERT_EQ(retrievedMetadata->contentMd5, metadata.contentMd5);
  ASSERT_EQ(retrievedMetadata->slotNumber, metadata.slotNumber);
  ASSERT_EQ(retrievedMetadata->savefileMd5, metadata.savefileMd5);
  ASSERT_EQ(retrievedMetadata->lastModifiedAt, metadata.lastModifiedAt);
  ASSERT_TRUE(retrievedMetadata->createdAt > 0);
}

TEST_F(SqliteUserdataDatabaseTest,
       CreateSavefileMetadataSupportsMultipleSlots) {
  SqliteUserdataDatabase db(temp_file_path.string());

  const auto result = db.getSavefileMetadataForContent("1234567890");
  ASSERT_TRUE(result.empty());

  SavefileMetadata metadata;
  metadata.id = -1;
  metadata.contentMd5 = "1234567890";
  metadata.slotNumber = 1;
  metadata.savefileMd5 = "0987654321";
  metadata.lastModifiedAt = 1000;

  ASSERT_TRUE(db.createSavefileMetadata(metadata));
  ASSERT_NE(metadata.id, -1);
  ASSERT_EQ(db.getSavefileMetadataForContent("1234567890").size(), 1);

  metadata.id = -1;
  metadata.slotNumber = 2;

  ASSERT_TRUE(db.createSavefileMetadata(metadata));
  ASSERT_NE(metadata.id, -1);
  ASSERT_EQ(db.getSavefileMetadataForContent("1234567890").size(), 2);
}

TEST_F(SqliteUserdataDatabaseTest,
       EnsureIDidntAccidentallyMakeItUniqueOnSlotNumber) {
  SqliteUserdataDatabase db(temp_file_path.string());

  const auto result = db.getSavefileMetadataForContent("1234567890");
  ASSERT_TRUE(result.empty());

  SavefileMetadata metadata;
  metadata.id = -1;
  metadata.contentMd5 = "1234567890";
  metadata.slotNumber = 1;
  metadata.savefileMd5 = "0987654321";
  metadata.lastModifiedAt = 1000;

  ASSERT_TRUE(db.createSavefileMetadata(metadata));
  ASSERT_NE(metadata.id, -1);
  ASSERT_EQ(db.getSavefileMetadataForContent("1234567890").size(), 1);

  metadata.id = -1;
  metadata.contentMd5 = "4444";

  ASSERT_TRUE(db.createSavefileMetadata(metadata));
  ASSERT_NE(metadata.id, -1);
  ASSERT_EQ(db.getSavefileMetadataForContent("4444").size(), 1);
}

TEST_F(SqliteUserdataDatabaseTest, UpdateSavefileMetadataTest) {
  SqliteUserdataDatabase db(temp_file_path.string());

  const auto result = db.getSavefileMetadataForContent("1234567890");
  ASSERT_TRUE(result.empty());

  SavefileMetadata metadata;
  metadata.id = -1;
  metadata.contentMd5 = "1234567890";
  metadata.slotNumber = 1;
  metadata.savefileMd5 = "0987654321";
  metadata.lastModifiedAt = 1000;

  ASSERT_TRUE(db.createSavefileMetadata(metadata));
  ASSERT_NE(metadata.id, -1);
  ASSERT_EQ(db.getSavefileMetadataForContent("1234567890").at(0).savefileMd5,
            "0987654321");

  auto id = metadata.id;

  metadata.savefileMd5 = "1111111111";
  ASSERT_TRUE(db.updateSavefileMetadata(metadata));

  auto updatedMetadata = db.getSavefileMetadata("1234567890", 1);

  ASSERT_TRUE(updatedMetadata.has_value());
  ASSERT_EQ(updatedMetadata->id, id);
  ASSERT_EQ(updatedMetadata->savefileMd5, "1111111111");
}

TEST_F(SqliteUserdataDatabaseTest, UpdateSavefileMetadataWhenNotExist) {
  SqliteUserdataDatabase db(temp_file_path.string());

  const auto result = db.getSavefileMetadataForContent("1234567890");
  ASSERT_TRUE(result.empty());

  SavefileMetadata metadata;
  metadata.id = 123;
  metadata.contentMd5 = "1234567890";
  metadata.slotNumber = 1;
  metadata.savefileMd5 = "0987654321";
  metadata.lastModifiedAt = 1000;
  metadata.createdAt = 500;

  ASSERT_FALSE(db.updateSavefileMetadata(metadata));
}

TEST_F(SqliteUserdataDatabaseTest, CreatePlaySessionTest) {
  SqliteUserdataDatabase db(temp_file_path.string());

  PlaySession session;
  session.id = -1;
  session.contentMd5 = "1234567890";
  session.slotNumber = 1;
  session.startTime = 1000;
  session.endTime = 2000;
  session.unpausedDurationSeconds = 500;

  ASSERT_TRUE(db.createPlaySession(session));
  ASSERT_NE(session.id, -1);
}
//
// TEST_F(SqliteUserdataDatabaseTest, GetOrCreateSavefileMetadataTest) {
//   SqliteUserdataDatabase db(temp_file_path.string());
//
//   // Add code to test the getOrCreateSavefileMetadata method
//   auto result = db.getOrCreateSavefileMetadata("1234567890", 1);
//   // Add assertions to check the state of the database after the operation
// }
//
// TEST_F(SqliteUserdataDatabaseTest, UpdateSavefileMetadataTest) {
//   SqliteUserdataDatabase db(temp_file_path.string());
//
//   // Add code to test the updateSavefileMetadata method
//   SavefileMetadata metadata;
//   metadata.contentMd5 = "1234567890";
//   metadata.slotNumber = 1;
//   metadata.savefileMd5 = "0987654321";
//   metadata.lastModifiedAt = 1000;
//   metadata.createdAt = 500;
//
//   db.updateSavefileMetadata(metadata);
//   // Add assertions to check the state of the database after the operation
// }
//
// TEST_F(SqliteUserdataDatabaseTest, CreateSavefileMetadataTest) {
//   SqliteUserdataDatabase db(temp_file_path.string());
//
//   // Add code to test the createSavefileMetadata method
//   SavefileMetadata metadata;
//   metadata.contentMd5 = "1234567890";
//   metadata.slotNumber = 1;
//   metadata.savefileMd5 = "0987654321";
//   metadata.lastModifiedAt = 1000;
//   metadata.createdAt = 500;
//
//   ASSERT_TRUE(db.createSavefileMetadata(metadata));
//   ASSERT_NE(metadata.id, -1);
// }

} // namespace firelight::db
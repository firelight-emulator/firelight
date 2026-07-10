#include <firelight/db/sqlite_userdata_database.hpp>
#include <gtest/gtest.h>
#include <filesystem>

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
  SqliteUserdataDatabase db(QString::fromStdString(temp_file_path.string()));

  // Add assertions to check the initial state of the database
  ASSERT_TRUE(db.tableExists("savefile_metadata"));
}

TEST_F(SqliteUserdataDatabaseTest, CreateSavefileMetadataSetsId) {
  SqliteUserdataDatabase db(QString::fromStdString(temp_file_path.string()));
  SavefileMetadata metadata;
  metadata.id = -1;
  metadata.contentId = "1234567890";
  metadata.slotNumber = 1;
  metadata.savefileMd5 = "0987654321";
  metadata.lastModifiedAt = 1000;

  ASSERT_TRUE(db.createSavefileMetadata(metadata));
  ASSERT_NE(metadata.id, -1);

  const auto playlists = db.getSavefileMetadataForContent(metadata.contentId);
  ASSERT_EQ(playlists.size(), 1);
  ASSERT_EQ(playlists[0].id, metadata.id);
  ASSERT_EQ(playlists[0].contentId, metadata.contentId);
  ASSERT_EQ(playlists[0].slotNumber, metadata.slotNumber);
  ASSERT_EQ(playlists[0].savefileMd5, metadata.savefileMd5);
  ASSERT_EQ(playlists[0].lastModifiedAt, metadata.lastModifiedAt);
  ASSERT_TRUE(playlists[0].createdAt > 0);
}

TEST_F(SqliteUserdataDatabaseTest, GetSavefileMetadataTest) {
  SqliteUserdataDatabase db(QString::fromStdString(temp_file_path.string()));

  const auto result = db.getSavefileMetadata("1234567890", 1);
  ASSERT_FALSE(result.has_value());

  SavefileMetadata metadata;
  metadata.id = -1;
  metadata.contentId = "1234567890";
  metadata.slotNumber = 1;
  metadata.savefileMd5 = "0987654321";
  metadata.lastModifiedAt = 1000;

  ASSERT_TRUE(db.createSavefileMetadata(metadata));
  ASSERT_NE(metadata.id, -1);

  const auto retrievedMetadata =
      db.getSavefileMetadata(metadata.contentId, metadata.slotNumber);
  ASSERT_TRUE(retrievedMetadata.has_value());
  ASSERT_EQ(retrievedMetadata->id, metadata.id);
  ASSERT_EQ(retrievedMetadata->contentId, metadata.contentId);
  ASSERT_EQ(retrievedMetadata->slotNumber, metadata.slotNumber);
  ASSERT_EQ(retrievedMetadata->savefileMd5, metadata.savefileMd5);
  ASSERT_EQ(retrievedMetadata->lastModifiedAt, metadata.lastModifiedAt);
  ASSERT_TRUE(retrievedMetadata->createdAt > 0);
}

TEST_F(SqliteUserdataDatabaseTest,
       CreateSavefileMetadataSupportsMultipleSlots) {
  SqliteUserdataDatabase db(QString::fromStdString(temp_file_path.string()));

  const auto result = db.getSavefileMetadataForContent("1234567890");
  ASSERT_TRUE(result.empty());

  SavefileMetadata metadata;
  metadata.id = -1;
  metadata.contentId = "1234567890";
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
  SqliteUserdataDatabase db(QString::fromStdString(temp_file_path.string()));

  const auto result = db.getSavefileMetadataForContent("1234567890");
  ASSERT_TRUE(result.empty());

  SavefileMetadata metadata;
  metadata.id = -1;
  metadata.contentId = "1234567890";
  metadata.slotNumber = 1;
  metadata.savefileMd5 = "0987654321";
  metadata.lastModifiedAt = 1000;

  ASSERT_TRUE(db.createSavefileMetadata(metadata));
  ASSERT_NE(metadata.id, -1);
  ASSERT_EQ(db.getSavefileMetadataForContent("1234567890").size(), 1);

  metadata.id = -1;
  metadata.contentId = "4444";

  ASSERT_TRUE(db.createSavefileMetadata(metadata));
  ASSERT_NE(metadata.id, -1);
  ASSERT_EQ(db.getSavefileMetadataForContent("4444").size(), 1);
}

TEST_F(SqliteUserdataDatabaseTest, UpdateSavefileMetadataTest) {
  SqliteUserdataDatabase db(QString::fromStdString(temp_file_path.string()));

  const auto result = db.getSavefileMetadataForContent("1234567890");
  ASSERT_TRUE(result.empty());

  SavefileMetadata metadata;
  metadata.id = -1;
  metadata.contentId = "1234567890";
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
  SqliteUserdataDatabase db(QString::fromStdString(temp_file_path.string()));

  const auto result = db.getSavefileMetadataForContent("1234567890");
  ASSERT_TRUE(result.empty());

  SavefileMetadata metadata;
  metadata.id = 123;
  metadata.contentId = "1234567890";
  metadata.slotNumber = 1;
  metadata.savefileMd5 = "0987654321";
  metadata.lastModifiedAt = 1000;
  metadata.createdAt = 500;

  ASSERT_FALSE(db.updateSavefileMetadata(metadata));
}

namespace {
ModInstallation makeInstallation(int entryId, const std::string &baseHash,
                                 const std::string &patchedHash) {
  ModInstallation installation;
  installation.entryId = entryId;
  installation.baseContentHash = baseHash;
  installation.modId = 42;
  installation.modSlug = "radical-red";
  installation.installedPatchId = 7;
  installation.installedVersion = "1.2.0";
  installation.saveGeneration = 1;
  installation.patchedContentHash = patchedHash;
  installation.patchFilePath = "/patches/radical-red-1.2.0.bps";
  installation.pinned = false;
  installation.source = "shop";
  return installation;
}
} // namespace

TEST_F(SqliteUserdataDatabaseTest, ConstructorCreatesModInstallationTable) {
  SqliteUserdataDatabase db(QString::fromStdString(temp_file_path.string()));
  ASSERT_TRUE(db.tableExists("mod_installation"));
}

TEST_F(SqliteUserdataDatabaseTest, CreateModInstallationSetsIdAndCreatedAt) {
  SqliteUserdataDatabase db(QString::fromStdString(temp_file_path.string()));
  auto installation = makeInstallation(10, "basehash", "patchedhash");

  ASSERT_TRUE(db.createModInstallation(installation));
  ASSERT_NE(installation.id, -1);
  ASSERT_GT(installation.createdAt, 0u);

  const auto fetched = db.getModInstallation(installation.id);
  ASSERT_TRUE(fetched.has_value());
  EXPECT_EQ(fetched->entryId, 10);
  EXPECT_EQ(fetched->baseContentHash, "basehash");
  EXPECT_EQ(fetched->modId, 42);
  EXPECT_EQ(fetched->modSlug, "radical-red");
  EXPECT_EQ(fetched->installedPatchId, 7);
  EXPECT_EQ(fetched->installedVersion, "1.2.0");
  EXPECT_EQ(fetched->saveGeneration, 1);
  EXPECT_EQ(fetched->patchedContentHash, "patchedhash");
  EXPECT_EQ(fetched->patchFilePath, "/patches/radical-red-1.2.0.bps");
  EXPECT_FALSE(fetched->pinned);
  EXPECT_EQ(fetched->source, "shop");
}

TEST_F(SqliteUserdataDatabaseTest, GetModInstallationForEntry) {
  SqliteUserdataDatabase db(QString::fromStdString(temp_file_path.string()));
  auto installation = makeInstallation(11, "basehash", "patchedhash");
  ASSERT_TRUE(db.createModInstallation(installation));

  const auto forEntry = db.getModInstallationForEntry(11);
  ASSERT_TRUE(forEntry.has_value());
  EXPECT_EQ(forEntry->id, installation.id);

  EXPECT_FALSE(db.getModInstallationForEntry(999).has_value());
}

TEST_F(SqliteUserdataDatabaseTest, GetModInstallationsForBaseContentHash) {
  SqliteUserdataDatabase db(QString::fromStdString(temp_file_path.string()));
  // Two mods on the same base game, one mod on a different base game.
  auto a = makeInstallation(1, "smw", "smw-kaizo");
  auto b = makeInstallation(2, "smw", "smw-star");
  auto c = makeInstallation(3, "othergame", "other-mod");
  ASSERT_TRUE(db.createModInstallation(a));
  ASSERT_TRUE(db.createModInstallation(b));
  ASSERT_TRUE(db.createModInstallation(c));

  const auto forSmw = db.getModInstallationsForBaseContentHash("smw");
  ASSERT_EQ(forSmw.size(), 2u);

  EXPECT_EQ(db.getModInstallationsForBaseContentHash("othergame").size(), 1u);
  EXPECT_TRUE(db.getModInstallationsForBaseContentHash("nope").empty());
  EXPECT_EQ(db.getAllModInstallations().size(), 3u);
}

TEST_F(SqliteUserdataDatabaseTest, UpdateModInstallationPersistsMutableFields) {
  SqliteUserdataDatabase db(QString::fromStdString(temp_file_path.string()));
  auto installation = makeInstallation(12, "basehash", "patched-v1");
  ASSERT_TRUE(db.createModInstallation(installation));

  // Simulate applying an update: new version, new patched hash, bumped save
  // generation, retained patch path, and the user pinning + skipping a version.
  installation.installedVersion = "1.3.0";
  installation.installedPatchId = 8;
  installation.saveGeneration = 2;
  installation.patchedContentHash = "patched-v2";
  installation.patchFilePath = "/patches/radical-red-1.3.0.bps";
  installation.pinned = true;
  installation.ignoreUpdatesUpToVersion = "1.3.0";
  ASSERT_TRUE(db.updateModInstallation(installation));

  const auto fetched = db.getModInstallation(installation.id);
  ASSERT_TRUE(fetched.has_value());
  EXPECT_EQ(fetched->installedVersion, "1.3.0");
  EXPECT_EQ(fetched->installedPatchId, 8);
  EXPECT_EQ(fetched->saveGeneration, 2);
  EXPECT_EQ(fetched->patchedContentHash, "patched-v2");
  EXPECT_EQ(fetched->patchFilePath, "/patches/radical-red-1.3.0.bps");
  EXPECT_TRUE(fetched->pinned);
  EXPECT_EQ(fetched->ignoreUpdatesUpToVersion, "1.3.0");
  // Immutable identity fields are unchanged.
  EXPECT_EQ(fetched->entryId, 12);
  EXPECT_EQ(fetched->baseContentHash, "basehash");
}

TEST_F(SqliteUserdataDatabaseTest, DeleteModInstallation) {
  SqliteUserdataDatabase db(QString::fromStdString(temp_file_path.string()));
  auto installation = makeInstallation(13, "basehash", "patchedhash");
  ASSERT_TRUE(db.createModInstallation(installation));

  ASSERT_TRUE(db.deleteModInstallation(installation.id));
  EXPECT_FALSE(db.getModInstallation(installation.id).has_value());
  EXPECT_FALSE(db.deleteModInstallation(installation.id));
}

TEST_F(SqliteUserdataDatabaseTest, OneModInstallationPerEntry) {
  SqliteUserdataDatabase db(QString::fromStdString(temp_file_path.string()));
  auto first = makeInstallation(20, "basehash", "patchedhash");
  ASSERT_TRUE(db.createModInstallation(first));

  // entry_id is unique: a second installation for the same entry is rejected.
  auto duplicate = makeInstallation(20, "basehash", "other");
  ASSERT_FALSE(db.createModInstallation(duplicate));
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
//   metadata.contentId = "1234567890";
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
//   metadata.contentId = "1234567890";
//   metadata.slotNumber = 1;
//   metadata.savefileMd5 = "0987654321";
//   metadata.lastModifiedAt = 1000;
//   metadata.createdAt = 500;
//
//   ASSERT_TRUE(db.createSavefileMetadata(metadata));
//   ASSERT_NE(metadata.id, -1);
// }

} // namespace firelight::db
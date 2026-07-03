#include <gtest/gtest.h>
#include <firelight/event_dispatcher.hpp>
#include <firelight/library/library_events.hpp>
#include <firelight/library/library_ingest_service.hpp>
#include <firelight/library/sqlite_user_library.hpp>

namespace firelight::db {
class SqliteUserLibraryTest : public testing::Test {};

// Counts the repository's content/run-configuration events (the EventDispatcher
// replacement for the old Qt signals). Subscriptions are released when it goes
// out of scope.
struct LibraryEventCounters {
  int contentFileAdded = 0;
  int runConfigCreated = 0;
  ScopedConnection contentFileAddedConn =
      EventDispatcher::instance().subscribe<library::ContentFileAddedEvent>(
          [this](const library::ContentFileAddedEvent &) {
            ++contentFileAdded;
          });
  ScopedConnection runConfigCreatedConn =
      EventDispatcher::instance()
          .subscribe<library::RunConfigurationCreatedEvent>(
              [this](const library::RunConfigurationCreatedEvent &) {
                ++runConfigCreated;
              });
};

TEST_F(SqliteUserLibraryTest, CreateFolderSetsIdTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::LibraryIngestService ingest(library);
  auto info = library::FolderInfo{.displayName = "test"};

  ASSERT_TRUE(library.create(info));
  ASSERT_NE(info.id, -1);
}

TEST_F(SqliteUserLibraryTest, CreateFolderWithExistingNameTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::LibraryIngestService ingest(library);
  auto info = library::FolderInfo{.displayName = "test"};

  ASSERT_TRUE(library.create(info));
  ASSERT_NE(info.id, -1);

  auto info2 = library::FolderInfo{.displayName = "test"};
  ASSERT_FALSE(library.create(info2));
  ASSERT_EQ(info2.id, -1);
}

TEST_F(SqliteUserLibraryTest, ListFoldersTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::LibraryIngestService ingest(library);

  ASSERT_TRUE(library.listFolders().empty());

  auto info = library::FolderInfo{.displayName = "test",
                                  .description = "test description",
                                  .iconSourceUrl = "testurl"};
  ASSERT_TRUE(library.create(info));

  const auto folders = library.listFolders();
  ASSERT_EQ(folders.size(), 1);
  ASSERT_EQ(folders[0].id, info.id);
  ASSERT_EQ(folders[0].displayName, info.displayName);
  ASSERT_EQ(folders[0].description, info.description);
  ASSERT_EQ(folders[0].iconSourceUrl, info.iconSourceUrl);
}

TEST_F(SqliteUserLibraryTest, UpdateFolderTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::LibraryIngestService ingest(library);

  auto info = library::FolderInfo{.displayName = "test"};
  ASSERT_TRUE(library.create(info));

  auto folderEntry =
      library::FolderEntryInfo{.folderId = info.id, .entryId = 1};
  ASSERT_TRUE(library.create(folderEntry));

  auto folders = library.listFolders();
  ASSERT_EQ(folders.size(), 1);
  ASSERT_EQ(folders[0].id, info.id);
  ASSERT_EQ(folders[0].displayName, info.displayName);
  ASSERT_EQ(folders[0].description, info.description);

  info.description = "test description";
  info.iconSourceUrl = "testurl";

  ASSERT_TRUE(library.update(info));

  folders = library.listFolders();
  ASSERT_EQ(folders.size(), 1);
  ASSERT_EQ(folders[0].id, info.id);
  ASSERT_EQ(folders[0].displayName, info.displayName);
  ASSERT_EQ(folders[0].description, info.description);
}

TEST_F(SqliteUserLibraryTest, UpdateFolderInvalidIdTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::LibraryIngestService ingest(library);

  auto info = library::FolderInfo{.displayName = "test"};
  ASSERT_TRUE(library.create(info));

  auto folderEntry =
      library::FolderEntryInfo{.folderId = info.id, .entryId = 1};
  ASSERT_TRUE(library.create(folderEntry));

  auto folders = library.listFolders();
  ASSERT_EQ(folders.size(), 1);

  info.id = -1; // Set to an invalid ID
  ASSERT_FALSE(library.update(info));
}

TEST_F(SqliteUserLibraryTest, UpdateFolderThatDoesntExistTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::LibraryIngestService ingest(library);

  ASSERT_EQ(0, library.listFolders().size());

  auto info = library::FolderInfo{.displayName = "test"};
  info.id = 1; // Set to an invalid ID
  ASSERT_FALSE(library.update(info));
}

TEST_F(SqliteUserLibraryTest, DeleteFolderTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::LibraryIngestService ingest(library);

  ASSERT_TRUE(library.listFolders().empty());

  auto info = library::FolderInfo{.displayName = "test"};
  ASSERT_TRUE(library.create(info));

  const auto folders = library.listFolders();
  ASSERT_EQ(folders.size(), 1);
  ASSERT_EQ(folders[0].id, info.id);

  ASSERT_TRUE(library.deleteFolder(info.id));
  ASSERT_TRUE(library.listFolders().empty());
}

TEST_F(SqliteUserLibraryTest, CreateFolderEntryTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::LibraryIngestService ingest(library);

  auto info = library::FolderInfo{.displayName = "test"};
  ASSERT_TRUE(library.create(info));

  auto folderEntry =
      library::FolderEntryInfo{.folderId = info.id, .entryId = 1};
  ASSERT_TRUE(library.create(folderEntry));
}

TEST_F(SqliteUserLibraryTest, DeleteFolderEntryTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::LibraryIngestService ingest(library);

  auto entry = library::Entry{
      .displayName = "test entry", .contentHash = "1234", .platformId = 1};

  ASSERT_TRUE(library.createEntry(entry));

  auto info = library::FolderInfo{.displayName = "test"};
  ASSERT_TRUE(library.create(info));

  auto folderEntry =
      library::FolderEntryInfo{.folderId = info.id, .entryId = entry.id};
  ASSERT_TRUE(library.create(folderEntry));

  auto actualEntry = library.getEntry(entry.id);
  ASSERT_TRUE(actualEntry.has_value());
  ASSERT_EQ(actualEntry->folderIds.at(0), info.id);

  // Now delete the folder entry
  ASSERT_TRUE(library.deleteFolderEntry(folderEntry));

  actualEntry = library.getEntry(entry.id);
  ASSERT_TRUE(actualEntry.has_value());
  ASSERT_EQ(actualEntry->folderIds.size(), 0);
}

TEST_F(SqliteUserLibraryTest, AddRomWithNoEntryTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::LibraryIngestService ingest(library);

  LibraryEventCounters counters;

  library::ContentFile romInfo{.m_fileSizeBytes = 123456,
                               .m_filePath = "test.rom",
                               .m_fileMd5 = "d41d8cd98f00b204e9800998ecf8427e",
                               .m_fileCrc32 = "12345678",
                               .m_inArchive = false,
                               .m_archivePathName = "",
                               .m_platformId = 1,
                               .m_contentHash =
                                   "d41d8cd98f00b204e9800998ecf8427e"};

  ASSERT_TRUE(library.create(romInfo));

  auto actualRomInfo = library.getContentFile(romInfo.m_id);
  ASSERT_TRUE(actualRomInfo.has_value());

  auto runConfigs = library.getRunConfigurations(
      QString::fromStdString(romInfo.m_contentHash));
  ASSERT_EQ(runConfigs.size(), 1);

  auto entry = library.getEntryWithContentHash(
      QString::fromStdString(romInfo.m_contentHash));
  ASSERT_TRUE(entry.has_value());
  ASSERT_FALSE(entry->hidden);

  ASSERT_EQ(counters.contentFileAdded, 1);
  ASSERT_EQ(counters.runConfigCreated, 1);
}

TEST_F(SqliteUserLibraryTest, AddRomWithExistingEntryTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::LibraryIngestService ingest(library);

  library::ContentFile romInfo{.m_fileSizeBytes = 123456,
                               .m_filePath = "test.rom",
                               .m_fileMd5 = "d41d8cd98f00b204e9800998ecf8427e",
                               .m_fileCrc32 = "12345678",
                               .m_inArchive = false,
                               .m_archivePathName = "",
                               .m_platformId = 1,
                               .m_contentHash =
                                   "d41d8cd98f00b204e9800998ecf8427e"};

  ASSERT_TRUE(library.create(romInfo));

  auto entry = library.getEntryWithContentHash(
      QString::fromStdString(romInfo.m_contentHash));
  ASSERT_TRUE(entry.has_value());
  ASSERT_FALSE(entry->hidden);

  // Count events after the initial entry is created so we don't track that one
  LibraryEventCounters counters;

  // Create with SAME content hash as existing entry
  library::ContentFile romInfo2{.m_fileSizeBytes = 1234567,
                                .m_filePath = "testNumberTwoBaby.rom",
                                .m_fileMd5 = "123456789abcdef0123456789abcdef",
                                .m_fileCrc32 = "12345678",
                                .m_inArchive = false,
                                .m_archivePathName = "",
                                .m_platformId = 1,
                                .m_contentHash =
                                    "d41d8cd98f00b204e9800998ecf8427e"};

  ASSERT_TRUE(library.create(romInfo2));

  auto actualRomInfo = library.getContentFile(romInfo.m_id);
  ASSERT_TRUE(actualRomInfo.has_value());

  auto runConfigs = library.getRunConfigurations(
      QString::fromStdString(romInfo.m_contentHash));
  ASSERT_EQ(runConfigs.size(), 2);

  auto actualEntry = library.getEntryWithContentHash(
      QString::fromStdString(romInfo.m_contentHash));
  ASSERT_EQ(entry->id, actualEntry->id);

  ASSERT_EQ(counters.contentFileAdded, 1);
  ASSERT_EQ(counters.runConfigCreated, 1);
}

TEST_F(SqliteUserLibraryTest, AddRomWithDuplicatePathTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::LibraryIngestService ingest(library);

  library::ContentFile romInfo{.m_fileSizeBytes = 123456,
                               .m_filePath = "test.rom",
                               .m_fileMd5 = "d41d8cd98f00b204e9800998ecf8427e",
                               .m_fileCrc32 = "12345678",
                               .m_inArchive = false,
                               .m_archivePathName = "",
                               .m_platformId = 1,
                               .m_contentHash =
                                   "d41d8cd98f00b204e9800998ecf8427e"};

  ASSERT_TRUE(library.create(romInfo));

  LibraryEventCounters counters;

  library::ContentFile newRomInfo{.m_fileSizeBytes = 123456,
                                  .m_filePath = "test.rom",
                                  .m_fileMd5 = "12344",
                                  .m_fileCrc32 = "12345678",
                                  .m_inArchive = false,
                                  .m_archivePathName = "",
                                  .m_platformId = 1,
                                  .m_contentHash = "1234"};

  ASSERT_FALSE(library.create(romInfo));
  ASSERT_EQ(counters.contentFileAdded, 0);
  ASSERT_EQ(counters.runConfigCreated, 0);
}

TEST_F(SqliteUserLibraryTest, DeleteRomForEntryWithMultipleRunConfigsTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::LibraryIngestService ingest(library);

  library::ContentFile romInfo{.m_fileSizeBytes = 123456,
                               .m_filePath = "test.rom",
                               .m_fileMd5 = "d41d8cd98f00b204e9800998ecf8427e",
                               .m_fileCrc32 = "12345678",
                               .m_inArchive = false,
                               .m_archivePathName = "",
                               .m_platformId = 1,
                               .m_contentHash =
                                   "d41d8cd98f00b204e9800998ecf8427e"};

  ASSERT_TRUE(library.create(romInfo));

  auto entry = library.getEntryWithContentHash(
      QString::fromStdString(romInfo.m_contentHash));
  ASSERT_TRUE(entry.has_value());
  ASSERT_FALSE(entry->hidden);

  // Create with SAME content hash as existing entry
  library::ContentFile romInfo2{.m_fileSizeBytes = 1234567,
                                .m_filePath = "testNumberTwoBaby.rom",
                                .m_fileMd5 = "123456789abcdef0123456789abcdef",
                                .m_fileCrc32 = "12345678",
                                .m_inArchive = false,
                                .m_archivePathName = "",
                                .m_platformId = 1,
                                .m_contentHash =
                                    "d41d8cd98f00b204e9800998ecf8427e"};

  ASSERT_TRUE(library.create(romInfo2));

  auto actualRomInfo = library.getContentFile(romInfo.m_id);
  ASSERT_TRUE(actualRomInfo.has_value());

  auto runConfigs = library.getRunConfigurations(
      QString::fromStdString(romInfo.m_contentHash));
  ASSERT_EQ(runConfigs.size(), 2);

  auto actualEntry = library.getEntryWithContentHash(
      QString::fromStdString(romInfo.m_contentHash));
  ASSERT_EQ(entry->id, actualEntry->id);

  ASSERT_TRUE(library.deleteContentFile(romInfo.m_id));

  // Get run configs again after deletion
  runConfigs = library.getRunConfigurations(
      QString::fromStdString(romInfo.m_contentHash));
  ASSERT_EQ(runConfigs.size(), 1);

  // Get entry again after deleting one rom
  actualEntry = library.getEntryWithContentHash(
      QString::fromStdString(romInfo.m_contentHash));
  ASSERT_TRUE(actualEntry.has_value());
  ASSERT_FALSE(actualEntry->hidden);
}

TEST_F(SqliteUserLibraryTest, DeleteRomHidesEntryTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::LibraryIngestService ingest(library);

  library::ContentFile romInfo{.m_fileSizeBytes = 123456,
                               .m_filePath = "test.rom",
                               .m_fileMd5 = "d41d8cd98f00b204e9800998ecf8427e",
                               .m_fileCrc32 = "12345678",
                               .m_inArchive = false,
                               .m_archivePathName = "",
                               .m_platformId = 1,
                               .m_contentHash =
                                   "d41d8cd98f00b204e9800998ecf8427e"};

  ASSERT_TRUE(library.create(romInfo));

  auto entry = library.getEntryWithContentHash(
      QString::fromStdString(romInfo.m_contentHash));
  ASSERT_TRUE(entry.has_value());
  ASSERT_FALSE(entry->hidden);

  auto actualEntry = library.getEntryWithContentHash(
      QString::fromStdString(romInfo.m_contentHash));
  ASSERT_EQ(entry->id, actualEntry->id);

  ASSERT_TRUE(library.deleteContentFile(romInfo.m_id));

  // Get entry again after deleting one rom
  actualEntry = library.getEntryWithContentHash(
      QString::fromStdString(romInfo.m_contentHash));
  ASSERT_TRUE(actualEntry.has_value());
  ASSERT_TRUE(actualEntry->hidden);
}

TEST_F(SqliteUserLibraryTest, AddingRomAfterDeletingUnhidesEntryTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::LibraryIngestService ingest(library);

  library::ContentFile romInfo{.m_fileSizeBytes = 123456,
                               .m_filePath = "test.rom",
                               .m_fileMd5 = "d41d8cd98f00b204e9800998ecf8427e",
                               .m_fileCrc32 = "12345678",
                               .m_inArchive = false,
                               .m_archivePathName = "",
                               .m_platformId = 1,
                               .m_contentHash =
                                   "d41d8cd98f00b204e9800998ecf8427e"};

  ASSERT_TRUE(library.create(romInfo));

  auto entry = library.getEntryWithContentHash(
      QString::fromStdString(romInfo.m_contentHash));
  ASSERT_TRUE(entry.has_value());
  ASSERT_FALSE(entry->hidden);

  auto actualEntry = library.getEntryWithContentHash(
      QString::fromStdString(romInfo.m_contentHash));
  ASSERT_EQ(entry->id, actualEntry->id);

  ASSERT_TRUE(library.deleteContentFile(romInfo.m_id));

  actualEntry = library.getEntryWithContentHash(
      QString::fromStdString(romInfo.m_contentHash));
  ASSERT_TRUE(actualEntry.has_value());
  ASSERT_TRUE(actualEntry->hidden);

  ASSERT_TRUE(library.create(romInfo));

  entry = library.getEntryWithContentHash(
      QString::fromStdString(romInfo.m_contentHash));
  ASSERT_TRUE(entry.has_value());
  ASSERT_FALSE(entry->hidden);
}

TEST_F(SqliteUserLibraryTest, RomsRemovedWhenContentDirectoryDeletedTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::LibraryIngestService ingest(library);

  // Create a content directory
  library::ContentDirectory main{.path = "test_content_directory"};
  ASSERT_TRUE(library.create(main));

  // Add a rom file to the content directory
  library::ContentFile romInfo{.m_fileSizeBytes = 123456,
                               .m_filePath = "test_content_directory/test.rom",
                               .m_fileMd5 = "d41d8cd98f00b204e9800998ecf8427e",
                               .m_fileCrc32 = "12345678",
                               .m_inArchive = false,
                               .m_archivePathName = "",
                               .m_platformId = 1,
                               .m_contentHash =
                                   "d41d8cd98f00b204e9800998ecf8427e"};

  ASSERT_TRUE(library.create(romInfo));

  // Verify the rom file was added
  auto actualRomInfo = library.getContentFile(romInfo.m_id);
  ASSERT_TRUE(actualRomInfo.has_value());

  // Delete the content directory
  ASSERT_TRUE(library.deleteContentDirectory(main.id));

  // Verify the rom file was removed
  actualRomInfo = library.getContentFile(romInfo.m_id);
  ASSERT_FALSE(actualRomInfo.has_value());
}

// TODO: Delete content directory, deletes all rom files in it, does the above

TEST_F(SqliteUserLibraryTest, UpdateEntryTest) {
  // TODO
}

TEST_F(SqliteUserLibraryTest, SmartFolderTypeAndFilterJsonRoundTripTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");

  auto info = library::FolderInfo{
      .displayName = "SNES favorites",
      .type = static_cast<int>(library::FolderType::Smart),
      .filterJson = R"({"platformIds":[3],"favorite":true})"};
  ASSERT_TRUE(library.create(info));

  auto folders = library.listFolders();
  ASSERT_EQ(folders.size(), 1);
  ASSERT_EQ(folders[0].type, static_cast<int>(library::FolderType::Smart));
  ASSERT_EQ(folders[0].filterJson, R"({"platformIds":[3],"favorite":true})");

  // A plain folder defaults to Manual with no criteria.
  auto manual = library::FolderInfo{.displayName = "Manual"};
  ASSERT_TRUE(library.create(manual));
  folders = library.listFolders();
  ASSERT_EQ(folders.size(), 2);
  for (const auto &f : folders) {
    if (f.displayName == "Manual") {
      ASSERT_EQ(f.type, static_cast<int>(library::FolderType::Manual));
      ASSERT_TRUE(f.filterJson.empty());
    }
  }

  // Updating criteria persists.
  info.filterJson = R"({"pathContains":"snes"})";
  ASSERT_TRUE(library.update(info));
  for (const auto &f : library.listFolders()) {
    if (f.id == info.id) {
      ASSERT_EQ(f.filterJson, R"({"pathContains":"snes"})");
    }
  }
}

TEST_F(SqliteUserLibraryTest, ContentDirectoryIdStampedOnCreateTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::LibraryIngestService ingest(library);

  library::ContentDirectory dir{.path = "roms"};
  ASSERT_TRUE(library.create(dir));

  library::ContentFile inDir{.m_fileSizeBytes = 100,
                             .m_filePath = "roms/game.sfc",
                             .m_fileMd5 = "aaa",
                             .m_fileCrc32 = "1",
                             .m_platformId = 1,
                             .m_contentHash = "hash-in-dir"};
  ASSERT_TRUE(library.create(inDir));
  auto stored = library.getContentFile(inDir.m_id);
  ASSERT_TRUE(stored.has_value());
  ASSERT_EQ(stored->m_contentDirectoryId, dir.id);

  // A file under no known directory gets -1.
  library::ContentFile orphan{.m_fileSizeBytes = 100,
                              .m_filePath = "elsewhere/game.sfc",
                              .m_fileMd5 = "bbb",
                              .m_fileCrc32 = "1",
                              .m_platformId = 1,
                              .m_contentHash = "hash-orphan"};
  ASSERT_TRUE(library.create(orphan));
  stored = library.getContentFile(orphan.m_id);
  ASSERT_TRUE(stored.has_value());
  ASSERT_EQ(stored->m_contentDirectoryId, -1);
}

TEST_F(SqliteUserLibraryTest, ContentDirectoryLongestPrefixWinsTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::LibraryIngestService ingest(library);

  library::ContentDirectory parent{.path = "roms"};
  ASSERT_TRUE(library.create(parent));
  library::ContentDirectory nested{.path = "roms/snes"};
  ASSERT_TRUE(library.create(nested));

  library::ContentFile file{.m_fileSizeBytes = 100,
                            .m_filePath = "roms/snes/game.sfc",
                            .m_fileMd5 = "ccc",
                            .m_fileCrc32 = "1",
                            .m_platformId = 1,
                            .m_contentHash = "hash-nested"};
  ASSERT_TRUE(library.create(file));
  auto stored = library.getContentFile(file.m_id);
  ASSERT_TRUE(stored.has_value());
  ASSERT_EQ(stored->m_contentDirectoryId, nested.id);
}

TEST_F(SqliteUserLibraryTest, EntryProvenancePopulatedTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::LibraryIngestService ingest(library);

  library::ContentDirectory dir{.path = "roms"};
  ASSERT_TRUE(library.create(dir));

  library::ContentFile file{.m_fileSizeBytes = 100,
                            .m_filePath = "roms/game.sfc",
                            .m_fileMd5 = "ddd",
                            .m_fileCrc32 = "1",
                            .m_platformId = 1,
                            .m_contentHash = "hash-entry"};
  ASSERT_TRUE(library.create(file));

  auto entry = library.getEntryWithContentHash("hash-entry");
  ASSERT_TRUE(entry.has_value());
  ASSERT_EQ(entry->contentDirectoryIds.size(), 1);
  ASSERT_EQ(entry->contentDirectoryIds.at(0), dir.id);
  ASSERT_EQ(entry->contentPaths.size(), 1);
  ASSERT_EQ(entry->contentPaths.at(0), "roms/game.sfc");
}

namespace {
// Finds a folder by id in a listFolders() result, or a default-constructed
// FolderInfo (id -1) if absent.
library::FolderInfo findFolder(const std::vector<library::FolderInfo> &folders,
                               int id) {
  for (const auto &f : folders) {
    if (f.id == id) {
      return f;
    }
  }
  return library::FolderInfo{};
}
} // namespace

TEST_F(SqliteUserLibraryTest, FolderColorAndSortRoundTripTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");

  auto info = library::FolderInfo{.displayName = "styled",
                                  .color = "#ff8800",
                                  .sortRole = "lastPlayedAt",
                                  .sortAscending = false};
  ASSERT_TRUE(library.create(info));

  auto stored = findFolder(library.listFolders(), info.id);
  ASSERT_EQ(stored.color, "#ff8800");
  ASSERT_EQ(stored.sortRole, "lastPlayedAt");
  ASSERT_FALSE(stored.sortAscending);

  // Update changes appearance.
  info.color = "#00ff00";
  info.sortRole = "displayName";
  info.sortAscending = true;
  ASSERT_TRUE(library.update(info));

  stored = findFolder(library.listFolders(), info.id);
  ASSERT_EQ(stored.color, "#00ff00");
  ASSERT_EQ(stored.sortRole, "displayName");
  ASSERT_TRUE(stored.sortAscending);
}

TEST_F(SqliteUserLibraryTest, FolderPositionAutoAssignedTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");

  auto a = library::FolderInfo{.displayName = "a"};
  auto b = library::FolderInfo{.displayName = "b"};
  auto c = library::FolderInfo{.displayName = "c"};
  ASSERT_TRUE(library.create(a));
  ASSERT_TRUE(library.create(b));
  ASSERT_TRUE(library.create(c));

  EXPECT_EQ(a.position, 0);
  EXPECT_EQ(b.position, 1);
  EXPECT_EQ(c.position, 2);

  // listFolders returns them in position order.
  auto folders = library.listFolders();
  ASSERT_EQ(folders.size(), 3);
  EXPECT_EQ(folders[0].id, a.id);
  EXPECT_EQ(folders[1].id, b.id);
  EXPECT_EQ(folders[2].id, c.id);
}

TEST_F(SqliteUserLibraryTest, ReorderFoldersTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");

  auto a = library::FolderInfo{.displayName = "a"};
  auto b = library::FolderInfo{.displayName = "b"};
  auto c = library::FolderInfo{.displayName = "c"};
  ASSERT_TRUE(library.create(a));
  ASSERT_TRUE(library.create(b));
  ASSERT_TRUE(library.create(c));

  // New order: c, a, b (parent -1 = root).
  ASSERT_TRUE(library.reorderFolders(-1, {c.id, a.id, b.id}));

  auto folders = library.listFolders();
  ASSERT_EQ(folders.size(), 3);
  EXPECT_EQ(folders[0].id, c.id);
  EXPECT_EQ(folders[1].id, a.id);
  EXPECT_EQ(folders[2].id, b.id);
  EXPECT_EQ(findFolder(folders, c.id).position, 0);
  EXPECT_EQ(findFolder(folders, a.id).position, 1);
  EXPECT_EQ(findFolder(folders, b.id).position, 2);
}

TEST_F(SqliteUserLibraryTest, SetFolderParentMovesAndAppendsTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");

  auto parent = library::FolderInfo{.displayName = "parent"};
  auto child = library::FolderInfo{.displayName = "child"};
  auto sibling = library::FolderInfo{.displayName = "sibling"};
  ASSERT_TRUE(library.create(parent));
  ASSERT_TRUE(library.create(child));
  ASSERT_TRUE(library.create(sibling));

  // Move both under parent; each appends at the end of parent's scope.
  ASSERT_TRUE(library.setFolderParent(child.id, parent.id));
  ASSERT_TRUE(library.setFolderParent(sibling.id, parent.id));

  auto folders = library.listFolders();
  const auto storedChild = findFolder(folders, child.id);
  const auto storedSibling = findFolder(folders, sibling.id);
  EXPECT_EQ(storedChild.parentId, parent.id);
  EXPECT_EQ(storedChild.position, 0);
  EXPECT_EQ(storedSibling.parentId, parent.id);
  EXPECT_EQ(storedSibling.position, 1);
  EXPECT_EQ(findFolder(folders, parent.id).parentId, -1);
}

TEST_F(SqliteUserLibraryTest, EntryProvenanceUsesArchivePathTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::LibraryIngestService ingest(library);

  library::ContentDirectory dir{.path = "roms"};
  ASSERT_TRUE(library.create(dir));

  // An archived entry's on-disk location is the archive path; provenance
  // resolves against that, and contentPaths reports it (so "path contains"
  // works for archived content too).
  library::ContentFile file{.m_fileSizeBytes = 100,
                            .m_filePath = "game.sfc",
                            .m_fileMd5 = "eee",
                            .m_fileCrc32 = "1",
                            .m_inArchive = true,
                            .m_archivePathName = "roms/games.zip",
                            .m_platformId = 1,
                            .m_contentHash = "hash-archive"};
  ASSERT_TRUE(library.create(file));

  auto stored = library.getContentFile(file.m_id);
  ASSERT_TRUE(stored.has_value());
  ASSERT_EQ(stored->m_contentDirectoryId, dir.id);

  auto entry = library.getEntryWithContentHash("hash-archive");
  ASSERT_TRUE(entry.has_value());
  ASSERT_EQ(entry->contentDirectoryIds.at(0), dir.id);
  ASSERT_EQ(entry->contentPaths.at(0), "roms/games.zip");
}

} // namespace firelight::db

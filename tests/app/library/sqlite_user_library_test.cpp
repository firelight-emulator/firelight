#include <gtest/gtest.h>
#include <library/sqlite_user_library.hpp>
#include <qsignalspy.h>

namespace firelight::db {
class SqliteUserLibraryTest : public testing::Test {};

TEST_F(SqliteUserLibraryTest, CreateFolderSetsIdTest) {
  auto library = library::SqliteUserLibrary(":memory:", "file.txt");
  auto info = library::FolderInfo{.displayName = "test"};

  ASSERT_TRUE(library.create(info));
  ASSERT_NE(info.id, -1);
}

TEST_F(SqliteUserLibraryTest, CreateFolderWithExistingNameTest) {
  auto library = library::SqliteUserLibrary(":memory:", "file.txt");
  auto info = library::FolderInfo{.displayName = "test"};

  ASSERT_TRUE(library.create(info));
  ASSERT_NE(info.id, -1);

  auto info2 = library::FolderInfo{.displayName = "test"};
  ASSERT_FALSE(library.create(info2));
  ASSERT_EQ(info2.id, -1);
}

TEST_F(SqliteUserLibraryTest, ListFoldersTest) {
  auto library = library::SqliteUserLibrary(":memory:", "file.txt");

  ASSERT_TRUE(library.listFolders({}).empty());

  auto info = library::FolderInfo{.displayName = "test",
                                  .description = "test description",
                                  .iconSourceUrl = "testurl"};
  ASSERT_TRUE(library.create(info));

  const auto folders = library.listFolders({});
  ASSERT_EQ(folders.size(), 1);
  ASSERT_EQ(folders[0].id, info.id);
  ASSERT_EQ(folders[0].displayName, info.displayName);
  ASSERT_EQ(folders[0].description, info.description);
  ASSERT_EQ(folders[0].iconSourceUrl, info.iconSourceUrl);
}

TEST_F(SqliteUserLibraryTest, UpdateFolderTest) {
  auto library = library::SqliteUserLibrary(":memory:", "file.txt");

  auto info = library::FolderInfo{.displayName = "test"};
  ASSERT_TRUE(library.create(info));

  auto folderEntry =
      library::FolderEntryInfo{.folderId = info.id, .entryId = 1};
  ASSERT_TRUE(library.create(folderEntry));

  auto folders = library.listFolders({});
  ASSERT_EQ(folders.size(), 1);
  ASSERT_EQ(folders[0].id, info.id);
  ASSERT_EQ(folders[0].displayName, info.displayName);
  ASSERT_EQ(folders[0].description, info.description);

  info.description = "test description";
  info.iconSourceUrl = "testurl";

  ASSERT_TRUE(library.update(info));

  folders = library.listFolders({});
  ASSERT_EQ(folders.size(), 1);
  ASSERT_EQ(folders[0].id, info.id);
  ASSERT_EQ(folders[0].displayName, info.displayName);
  ASSERT_EQ(folders[0].description, info.description);
}

TEST_F(SqliteUserLibraryTest, UpdateFolderInvalidIdTest) {
  auto library = library::SqliteUserLibrary(":memory:", "file.txt");

  auto info = library::FolderInfo{.displayName = "test"};
  ASSERT_TRUE(library.create(info));

  auto folderEntry =
      library::FolderEntryInfo{.folderId = info.id, .entryId = 1};
  ASSERT_TRUE(library.create(folderEntry));

  auto folders = library.listFolders({});
  ASSERT_EQ(folders.size(), 1);

  info.id = -1; // Set to an invalid ID
  ASSERT_FALSE(library.update(info));
}

TEST_F(SqliteUserLibraryTest, UpdateFolderThatDoesntExistTest) {
  auto library = library::SqliteUserLibrary(":memory:", "file.txt");

  ASSERT_EQ(0, library.listFolders({}).size());

  auto info = library::FolderInfo{.displayName = "test"};
  info.id = 1; // Set to an invalid ID
  ASSERT_FALSE(library.update(info));
}

TEST_F(SqliteUserLibraryTest, DeleteFolderTest) {
  auto library = library::SqliteUserLibrary(":memory:", "file.txt");

  ASSERT_TRUE(library.listFolders({}).empty());

  auto info = library::FolderInfo{.displayName = "test"};
  ASSERT_TRUE(library.create(info));

  const auto folders = library.listFolders({});
  ASSERT_EQ(folders.size(), 1);
  ASSERT_EQ(folders[0].id, info.id);

  ASSERT_TRUE(library.deleteFolder(info.id));
  ASSERT_TRUE(library.listFolders({}).empty());
}

TEST_F(SqliteUserLibraryTest, CreateFolderEntryTest) {
  auto library = library::SqliteUserLibrary(":memory:", "file.txt");

  auto info = library::FolderInfo{.displayName = "test"};
  ASSERT_TRUE(library.create(info));

  auto folderEntry =
      library::FolderEntryInfo{.folderId = info.id, .entryId = 1};
  ASSERT_TRUE(library.create(folderEntry));
}

TEST_F(SqliteUserLibraryTest, DeleteFolderEntryTest) {
  auto library = library::SqliteUserLibrary(":memory:", "file.txt");

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
  auto library = library::SqliteUserLibrary(":memory:", "file.txt");

  QSignalSpy romAdded(&library, &library::SqliteUserLibrary::romFileAdded);
  QSignalSpy runConfigAdded(
      &library, &library::SqliteUserLibrary::romRunConfigurationCreated);
  QSignalSpy entryCreated(&library, &library::SqliteUserLibrary::entryCreated);

  library::RomFileInfo romInfo{.m_filePath = "test.rom",
                               .m_fileSizeBytes = 123456,
                               .m_fileMd5 = "d41d8cd98f00b204e9800998ecf8427e",
                               .m_fileCrc32 = "12345678",
                               .m_inArchive = false,
                               .m_archivePathName = "",
                               .m_platformId = 1,
                               .m_contentHash =
                                   "d41d8cd98f00b204e9800998ecf8427e"};

  ASSERT_TRUE(library.create(romInfo));

  auto actualRomInfo = library.getRomFile(romInfo.m_id);
  ASSERT_TRUE(actualRomInfo.has_value());

  auto runConfigs = library.getRunConfigurations(
      QString::fromStdString(romInfo.m_contentHash));
  ASSERT_EQ(runConfigs.size(), 1);

  auto entry = library.getEntryWithContentHash(
      QString::fromStdString(romInfo.m_contentHash));
  ASSERT_TRUE(entry.has_value());
  ASSERT_FALSE(entry->hidden);

  ASSERT_EQ(romAdded.count(), 1);
  ASSERT_EQ(runConfigAdded.count(), 1);
  ASSERT_EQ(entryCreated.count(), 1);
}

TEST_F(SqliteUserLibraryTest, AddRomWithExistingEntryTest) {
  auto library = library::SqliteUserLibrary(":memory:", "file.txt");

  library::RomFileInfo romInfo{.m_filePath = "test.rom",
                               .m_fileSizeBytes = 123456,
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

  // Create signal spies after the initial entry is created so we don't track
  // that one
  QSignalSpy romAdded(&library, &library::SqliteUserLibrary::romFileAdded);
  QSignalSpy runConfigAdded(
      &library, &library::SqliteUserLibrary::romRunConfigurationCreated);
  QSignalSpy entryCreated(&library, &library::SqliteUserLibrary::entryCreated);

  // Create with SAME content hash as existing entry
  library::RomFileInfo romInfo2{.m_filePath = "testNumberTwoBaby.rom",
                                .m_fileSizeBytes = 1234567,
                                .m_fileMd5 = "123456789abcdef0123456789abcdef",
                                .m_fileCrc32 = "12345678",
                                .m_inArchive = false,
                                .m_archivePathName = "",
                                .m_platformId = 1,
                                .m_contentHash =
                                    "d41d8cd98f00b204e9800998ecf8427e"};

  ASSERT_TRUE(library.create(romInfo2));

  auto actualRomInfo = library.getRomFile(romInfo.m_id);
  ASSERT_TRUE(actualRomInfo.has_value());

  auto runConfigs = library.getRunConfigurations(
      QString::fromStdString(romInfo.m_contentHash));
  ASSERT_EQ(runConfigs.size(), 2);

  auto actualEntry = library.getEntryWithContentHash(
      QString::fromStdString(romInfo.m_contentHash));
  ASSERT_EQ(entry->id, actualEntry->id);

  ASSERT_EQ(romAdded.count(), 1);
  ASSERT_EQ(runConfigAdded.count(), 1);
  ASSERT_EQ(entryCreated.count(), 0); // Entry should already exist, no new
                                      // entry should be created
}

TEST_F(SqliteUserLibraryTest, AddRomWithDuplicatePathTest) {
  auto library = library::SqliteUserLibrary(":memory:", "file.txt");

  library::RomFileInfo romInfo{.m_filePath = "test.rom",
                               .m_fileSizeBytes = 123456,
                               .m_fileMd5 = "d41d8cd98f00b204e9800998ecf8427e",
                               .m_fileCrc32 = "12345678",
                               .m_inArchive = false,
                               .m_archivePathName = "",
                               .m_platformId = 1,
                               .m_contentHash =
                                   "d41d8cd98f00b204e9800998ecf8427e"};

  ASSERT_TRUE(library.create(romInfo));

  QSignalSpy romAdded(&library, &library::SqliteUserLibrary::romFileAdded);
  QSignalSpy runConfigAdded(
      &library, &library::SqliteUserLibrary::romRunConfigurationCreated);
  QSignalSpy entryCreated(&library, &library::SqliteUserLibrary::entryCreated);

  library::RomFileInfo newRomInfo{.m_filePath = "test.rom",
                                  .m_fileSizeBytes = 123456,
                                  .m_fileMd5 = "12344",
                                  .m_fileCrc32 = "12345678",
                                  .m_inArchive = false,
                                  .m_archivePathName = "",
                                  .m_platformId = 1,
                                  .m_contentHash = "1234"};

  ASSERT_FALSE(library.create(romInfo));
  ASSERT_EQ(romAdded.count(), 0);
  ASSERT_EQ(runConfigAdded.count(), 0);
  ASSERT_EQ(entryCreated.count(), 0);
}

TEST_F(SqliteUserLibraryTest, DeleteRomForEntryWithMultipleRunConfigsTest) {
  auto library = library::SqliteUserLibrary(":memory:", "file.txt");

  library::RomFileInfo romInfo{.m_filePath = "test.rom",
                               .m_fileSizeBytes = 123456,
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
  library::RomFileInfo romInfo2{.m_filePath = "testNumberTwoBaby.rom",
                                .m_fileSizeBytes = 1234567,
                                .m_fileMd5 = "123456789abcdef0123456789abcdef",
                                .m_fileCrc32 = "12345678",
                                .m_inArchive = false,
                                .m_archivePathName = "",
                                .m_platformId = 1,
                                .m_contentHash =
                                    "d41d8cd98f00b204e9800998ecf8427e"};

  ASSERT_TRUE(library.create(romInfo2));

  auto actualRomInfo = library.getRomFile(romInfo.m_id);
  ASSERT_TRUE(actualRomInfo.has_value());

  auto runConfigs = library.getRunConfigurations(
      QString::fromStdString(romInfo.m_contentHash));
  ASSERT_EQ(runConfigs.size(), 2);

  auto actualEntry = library.getEntryWithContentHash(
      QString::fromStdString(romInfo.m_contentHash));
  ASSERT_EQ(entry->id, actualEntry->id);

  ASSERT_TRUE(library.deleteRomFile(romInfo.m_id));

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
  auto library = library::SqliteUserLibrary(":memory:", "file.txt");

  library::RomFileInfo romInfo{.m_filePath = "test.rom",
                               .m_fileSizeBytes = 123456,
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

  ASSERT_TRUE(library.deleteRomFile(romInfo.m_id));

  // Get entry again after deleting one rom
  actualEntry = library.getEntryWithContentHash(
      QString::fromStdString(romInfo.m_contentHash));
  ASSERT_TRUE(actualEntry.has_value());
  ASSERT_TRUE(actualEntry->hidden);
}

TEST_F(SqliteUserLibraryTest, AddingRomAfterDeletingUnhidesEntryTest) {
  auto library = library::SqliteUserLibrary(":memory:", "file.txt");

  library::RomFileInfo romInfo{.m_filePath = "test.rom",
                               .m_fileSizeBytes = 123456,
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

  ASSERT_TRUE(library.deleteRomFile(romInfo.m_id));

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
  auto library = library::SqliteUserLibrary(":memory:", "file.txt");

  // Create a content directory
  library::WatchedDirectory main{.path = "test_content_directory"};
  ASSERT_TRUE(library.create(main));

  // Add a rom file to the content directory
  library::RomFileInfo romInfo{.m_filePath = "test_content_directory/test.rom",
                               .m_fileSizeBytes = 123456,
                               .m_fileMd5 = "d41d8cd98f00b204e9800998ecf8427e",
                               .m_fileCrc32 = "12345678",
                               .m_inArchive = false,
                               .m_archivePathName = "",
                               .m_platformId = 1,
                               .m_contentHash =
                                   "d41d8cd98f00b204e9800998ecf8427e"};

  ASSERT_TRUE(library.create(romInfo));

  // Verify the rom file was added
  auto actualRomInfo = library.getRomFile(romInfo.m_id);
  ASSERT_TRUE(actualRomInfo.has_value());

  // Delete the content directory
  ASSERT_TRUE(library.deleteContentDirectory(main.id));

  // Verify the rom file was removed
  actualRomInfo = library.getRomFile(romInfo.m_id);
  ASSERT_FALSE(actualRomInfo.has_value());
}

// TODO: Delete content directory, deletes all rom files in it, does the above

TEST_F(SqliteUserLibraryTest, UpdateEntryTest) {
  // TODO
}

} // namespace firelight::db

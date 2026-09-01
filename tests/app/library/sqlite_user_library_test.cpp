// TODO: NEEDS REVIEW
#include <firelight/event_dispatcher.hpp>
#include <firelight/library/disc_set_service.hpp>
#include <firelight/library/library_events.hpp>
#include <firelight/library/library_ingest_service.hpp>
#include <firelight/library/sqlite_user_library.hpp>

#include <gtest/gtest.h>

namespace firelight::db {
class SqliteUserLibraryTest : public testing::Test {};

// Counts the repository's content/run-configuration events (the EventDispatcher
// replacement for the old Qt signals). Subscriptions are released when it goes
// out of scope
struct LibraryEventCounters {
  int contentFileAdded = 0;
  int runConfigCreated = 0;
  ScopedConnection contentFileAddedConn = EventDispatcher::instance().subscribe<library::ContentFileAddedEvent>(
      [this](const library::ContentFileAddedEvent &) { ++contentFileAdded; });
  ScopedConnection runConfigCreatedConn = EventDispatcher::instance().subscribe<library::RunConfigurationCreatedEvent>(
      [this](const library::RunConfigurationCreatedEvent &) { ++runConfigCreated; });
};

TEST_F(SqliteUserLibraryTest, CreateFolderSetsIdTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::DiscSetService discSets(library, "");
  library::LibraryIngestService ingest(library, discSets);
  auto info = library::FolderInfo{.displayName = "test"};

  ASSERT_TRUE(library.create(info));
  ASSERT_NE(info.id, -1);
}

TEST_F(SqliteUserLibraryTest, CreateFolderWithExistingNameTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::DiscSetService discSets(library, "");
  library::LibraryIngestService ingest(library, discSets);
  auto info = library::FolderInfo{.displayName = "test"};

  ASSERT_TRUE(library.create(info));
  ASSERT_NE(info.id, -1);

  auto info2 = library::FolderInfo{.displayName = "test"};
  ASSERT_FALSE(library.create(info2));
  ASSERT_EQ(info2.id, -1);
}

TEST_F(SqliteUserLibraryTest, ListFoldersTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::DiscSetService discSets(library, "");
  library::LibraryIngestService ingest(library, discSets);

  ASSERT_TRUE(library.listFolders().empty());

  auto info = library::FolderInfo{.displayName = "test", .description = "test description", .iconSourceUrl = "testurl"};
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
  library::DiscSetService discSets(library, "");
  library::LibraryIngestService ingest(library, discSets);

  auto info = library::FolderInfo{.displayName = "test"};
  ASSERT_TRUE(library.create(info));

  auto folderEntry = library::FolderEntry{.folderId = info.id, .entryId = 1};
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
  library::DiscSetService discSets(library, "");
  library::LibraryIngestService ingest(library, discSets);

  auto info = library::FolderInfo{.displayName = "test"};
  ASSERT_TRUE(library.create(info));

  auto folderEntry = library::FolderEntry{.folderId = info.id, .entryId = 1};
  ASSERT_TRUE(library.create(folderEntry));

  auto folders = library.listFolders();
  ASSERT_EQ(folders.size(), 1);

  info.id = -1; // Set to an invalid ID
  ASSERT_FALSE(library.update(info));
}

TEST_F(SqliteUserLibraryTest, UpdateFolderThatDoesntExistTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::DiscSetService discSets(library, "");
  library::LibraryIngestService ingest(library, discSets);

  ASSERT_EQ(0, library.listFolders().size());

  auto info = library::FolderInfo{.displayName = "test"};
  info.id = 1; // Set to an invalid ID
  ASSERT_FALSE(library.update(info));
}

TEST_F(SqliteUserLibraryTest, DeleteFolderTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::DiscSetService discSets(library, "");
  library::LibraryIngestService ingest(library, discSets);

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
  library::DiscSetService discSets(library, "");
  library::LibraryIngestService ingest(library, discSets);

  auto info = library::FolderInfo{.displayName = "test"};
  ASSERT_TRUE(library.create(info));

  auto folderEntry = library::FolderEntry{.folderId = info.id, .entryId = 1};
  ASSERT_TRUE(library.create(folderEntry));
}

// The scanner never re-derives a hand-set disc number, and nothing re-stamps set membership, so
// forgetting the row is how a file going away and coming back loses both
TEST_F(SqliteUserLibraryTest, AFileThatGoesAwayAndComesBackKeepsWhatWasSaidAboutIt) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::DiscSetService discSets(library, "");
  library::LibraryIngestService ingest(library, discSets);

  auto set = library::DiscSet{.title = "Final Fantasy VII", .discCount = 2};
  ASSERT_TRUE(library.createDiscSet(set));

  auto file = library::ContentFile{.m_type = library::ContentType::Disc,
                                   .m_fileSizeBytes = 4096,
                                   .m_filePath = "/roms/ff7-disc2.chd",
                                   .m_fileMd5 = "md5",
                                   .m_platformId = 8,
                                   .m_contentHash = "disc2",
                                   .m_discNumber = 2};
  ASSERT_TRUE(library.create(file));
  ASSERT_NE(file.m_id, -1);

  library::DiscSetMember member{
      .m_discSetId = set.id, .m_discNumber = 2, .m_contentFileId = file.m_id, .m_memberPath = file.m_filePath};
  ASSERT_TRUE(library.create(member));

  const auto configsBefore = library.getRunConfigurations("disc2");
  ASSERT_EQ(configsBefore.size(), 1u) << "the file was catalogued without a way in";

  ASSERT_TRUE(library.markContentFileMissing(file.m_id));

  const auto whileGone = library.getContentFile(file.m_id);
  ASSERT_TRUE(whileGone.has_value()) << "the row was destroyed rather than marked";
  EXPECT_NE(whileGone->m_missingSince, 0);
  EXPECT_EQ(whileGone->m_filePath, "/roms/ff7-disc2.chd") << "the path is the whole point";
  EXPECT_EQ(whileGone->m_discNumber, 2);
  ASSERT_TRUE(library.getDiscSetMemberForContentFile(file.m_id).has_value()) << "membership went with the bytes";
  EXPECT_EQ(library.getRunConfigurations("disc2").size(), 1u) << "the way in went with the file";

  ASSERT_TRUE(library.reviveContentFile(file.m_id));

  const auto back = library.getContentFile(file.m_id);
  ASSERT_TRUE(back.has_value());
  EXPECT_EQ(back->m_missingSince, 0);

  const auto stillAMember = library.getDiscSetMemberForContentFile(file.m_id);
  ASSERT_TRUE(stillAMember.has_value());
  EXPECT_EQ(stillAMember->m_discSetId, set.id);
}

// Launching through a patch is a choice somebody made, and re-cataloguing the file gives it back
// a plain way in with no patch on it
TEST_F(SqliteUserLibraryTest, AFileThatGoesAwayAndComesBackKeepsTheWayInItLaunchesThrough) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::DiscSetService discSets(library, "");
  library::LibraryIngestService ingest(library, discSets);

  auto file = library::ContentFile{
      .m_fileSizeBytes = 1024, .m_filePath = "/roms/Sonic.md", .m_platformId = 5, .m_contentHash = "sonic"};
  ASSERT_TRUE(library.create(file));

  library.createRunConfiguration(file.m_id, file.m_filePath, file.m_platformId, "sonic");

  const auto waysIn = [&] { return library.getRunConfigurations("sonic").size(); };
  ASSERT_EQ(waysIn(), 1u) << "the way in was never created";

  ASSERT_TRUE(library.markContentFileMissing(file.m_id));
  EXPECT_EQ(waysIn(), 1u) << "the file going away took its way in with it";

  ASSERT_TRUE(library.reviveContentFile(file.m_id));
  EXPECT_EQ(waysIn(), 1u) << "the file came back without the way in it launched through";
  EXPECT_TRUE(library.getRunConfigurations("sonic").front().isDefault);
}

// One dump in two folders is one game with two ways in, and either one being readable is enough
TEST_F(SqliteUserLibraryTest, AGameWithTwoCopiesStaysOnTheShelfUntilBothAreGone) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::DiscSetService discSets(library, "");
  library::LibraryIngestService ingest(library, discSets);

  auto onDrive = library::ContentFile{.m_fileSizeBytes = 2048,
                                      .m_filePath = "E:/Games/Kirby.gb",
                                      .m_fileMd5 = "md5",
                                      .m_platformId = 4,
                                      .m_contentHash = "kirby"};
  ASSERT_TRUE(library.create(onDrive));

  auto backup = library::ContentFile{.m_fileSizeBytes = 2048,
                                     .m_filePath = "C:/Backup/Kirby.gb",
                                     .m_fileMd5 = "md5",
                                     .m_platformId = 4,
                                     .m_contentHash = "kirby"};
  ASSERT_TRUE(library.create(backup));

  const auto entryId = library.getEntryWithContentHash("kirby")->id;
  ASSERT_TRUE(library.getEntry(entryId)->isContentAvailable);

  ASSERT_TRUE(library.markContentFileMissing(onDrive.m_id));
  EXPECT_TRUE(library.getEntry(entryId)->isContentAvailable) << "one copy going took the game off the shelf";

  ASSERT_TRUE(library.markContentFileMissing(backup.m_id));
  EXPECT_FALSE(library.getEntry(entryId)->isContentAvailable)
      << "every copy is gone and the game still reads as playable";

  ASSERT_TRUE(library.reviveContentFile(backup.m_id));
  EXPECT_TRUE(library.getEntry(entryId)->isContentAvailable) << "a copy came back and the game stayed off the shelf";
}

// Whether a game is hidden is decided by whether its files are there, which the GUI does not know
// and holds a stale answer to. Writing it as part of a whole row would hand that stale answer back
TEST_F(SqliteUserLibraryTest, EditingAnEntryFromAStaleCopyDoesNotResurrectItsOldHiddenFlag) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::DiscSetService discSets(library, "");
  library::LibraryIngestService ingest(library, discSets);

  auto entry = library::Entry{.displayName = "Chrono Trigger", .contentHash = "1234", .platformId = 1};
  ASSERT_TRUE(library.createEntry(entry));

  auto stale = library.getEntry(entry.id);
  ASSERT_TRUE(stale.has_value());
  ASSERT_FALSE(stale->hidden);

  ASSERT_TRUE(library.setEntryHidden(entry.id, true));

  stale->favorite = true;
  ASSERT_TRUE(library.update(*stale));

  const auto stored = library.getEntry(entry.id);
  ASSERT_TRUE(stored.has_value());
  EXPECT_TRUE(stored->hidden) << "a favorite written from a stale copy put the entry back on the shelf";
  EXPECT_TRUE(stored->favorite) << "the field the caller actually meant to write did not land";
}

TEST_F(SqliteUserLibraryTest, DeleteFolderEntryTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::DiscSetService discSets(library, "");
  library::LibraryIngestService ingest(library, discSets);

  auto entry = library::Entry{.displayName = "test entry", .contentHash = "1234", .platformId = 1};

  ASSERT_TRUE(library.createEntry(entry));

  auto info = library::FolderInfo{.displayName = "test"};
  ASSERT_TRUE(library.create(info));

  auto folderEntry = library::FolderEntry{.folderId = info.id, .entryId = entry.id};
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
  library::DiscSetService discSets(library, "");
  library::LibraryIngestService ingest(library, discSets);

  LibraryEventCounters counters;

  library::ContentFile romInfo{.m_fileSizeBytes = 123456,
                               .m_filePath = "test.rom",
                               .m_fileMd5 = "d41d8cd98f00b204e9800998ecf8427e",
                               .m_fileCrc32 = "12345678",
                               .m_inArchive = false,
                               .m_archivePathName = "",
                               .m_platformId = 1,
                               .m_contentHash = "d41d8cd98f00b204e9800998ecf8427e"};

  ASSERT_TRUE(library.create(romInfo));

  auto actualRomInfo = library.getContentFile(romInfo.m_id);
  ASSERT_TRUE(actualRomInfo.has_value());

  auto runConfigs = library.getRunConfigurations(romInfo.m_contentHash);
  ASSERT_EQ(runConfigs.size(), 1);

  auto entry = library.getEntryWithContentHash(romInfo.m_contentHash);
  ASSERT_TRUE(entry.has_value());
  ASSERT_TRUE(entry->isContentAvailable);

  ASSERT_EQ(counters.contentFileAdded, 1);
  ASSERT_EQ(counters.runConfigCreated, 1);
}

TEST_F(SqliteUserLibraryTest, AddRomWithExistingEntryTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::DiscSetService discSets(library, "");
  library::LibraryIngestService ingest(library, discSets);

  library::ContentFile romInfo{.m_fileSizeBytes = 123456,
                               .m_filePath = "test.rom",
                               .m_fileMd5 = "d41d8cd98f00b204e9800998ecf8427e",
                               .m_fileCrc32 = "12345678",
                               .m_inArchive = false,
                               .m_archivePathName = "",
                               .m_platformId = 1,
                               .m_contentHash = "d41d8cd98f00b204e9800998ecf8427e"};

  ASSERT_TRUE(library.create(romInfo));

  auto entry = library.getEntryWithContentHash(romInfo.m_contentHash);
  ASSERT_TRUE(entry.has_value());
  ASSERT_TRUE(entry->isContentAvailable);

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
                                .m_contentHash = "d41d8cd98f00b204e9800998ecf8427e"};

  ASSERT_TRUE(library.create(romInfo2));

  auto actualRomInfo = library.getContentFile(romInfo.m_id);
  ASSERT_TRUE(actualRomInfo.has_value());

  auto runConfigs = library.getRunConfigurations(romInfo.m_contentHash);
  ASSERT_EQ(runConfigs.size(), 2);

  auto actualEntry = library.getEntryWithContentHash(romInfo.m_contentHash);
  ASSERT_EQ(entry->id, actualEntry->id);

  ASSERT_EQ(counters.contentFileAdded, 1);
  ASSERT_EQ(counters.runConfigCreated, 1);
}

TEST_F(SqliteUserLibraryTest, AddRomWithDuplicatePathTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::DiscSetService discSets(library, "");
  library::LibraryIngestService ingest(library, discSets);

  library::ContentFile romInfo{.m_fileSizeBytes = 123456,
                               .m_filePath = "test.rom",
                               .m_fileMd5 = "d41d8cd98f00b204e9800998ecf8427e",
                               .m_fileCrc32 = "12345678",
                               .m_inArchive = false,
                               .m_archivePathName = "",
                               .m_platformId = 1,
                               .m_contentHash = "d41d8cd98f00b204e9800998ecf8427e"};

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
  library::DiscSetService discSets(library, "");
  library::LibraryIngestService ingest(library, discSets);

  library::ContentFile romInfo{.m_fileSizeBytes = 123456,
                               .m_filePath = "test.rom",
                               .m_fileMd5 = "d41d8cd98f00b204e9800998ecf8427e",
                               .m_fileCrc32 = "12345678",
                               .m_inArchive = false,
                               .m_archivePathName = "",
                               .m_platformId = 1,
                               .m_contentHash = "d41d8cd98f00b204e9800998ecf8427e"};

  ASSERT_TRUE(library.create(romInfo));

  auto entry = library.getEntryWithContentHash(romInfo.m_contentHash);
  ASSERT_TRUE(entry.has_value());
  ASSERT_TRUE(entry->isContentAvailable);

  // Create with SAME content hash as existing entry
  library::ContentFile romInfo2{.m_fileSizeBytes = 1234567,
                                .m_filePath = "testNumberTwoBaby.rom",
                                .m_fileMd5 = "123456789abcdef0123456789abcdef",
                                .m_fileCrc32 = "12345678",
                                .m_inArchive = false,
                                .m_archivePathName = "",
                                .m_platformId = 1,
                                .m_contentHash = "d41d8cd98f00b204e9800998ecf8427e"};

  ASSERT_TRUE(library.create(romInfo2));

  auto actualRomInfo = library.getContentFile(romInfo.m_id);
  ASSERT_TRUE(actualRomInfo.has_value());

  auto runConfigs = library.getRunConfigurations(romInfo.m_contentHash);
  ASSERT_EQ(runConfigs.size(), 2);

  auto actualEntry = library.getEntryWithContentHash(romInfo.m_contentHash);
  ASSERT_EQ(entry->id, actualEntry->id);

  ASSERT_TRUE(library.deleteContentFile(romInfo.m_id));

  // Get run configs again after deletion
  runConfigs = library.getRunConfigurations(romInfo.m_contentHash);
  ASSERT_EQ(runConfigs.size(), 1);

  // Get entry again after deleting one rom
  actualEntry = library.getEntryWithContentHash(romInfo.m_contentHash);
  ASSERT_TRUE(actualEntry.has_value());
  ASSERT_FALSE(actualEntry->hidden);
}

TEST_F(SqliteUserLibraryTest, DeleteRomLeavesEntryUnavailableTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::DiscSetService discSets(library, "");
  library::LibraryIngestService ingest(library, discSets);

  library::ContentFile romInfo{.m_fileSizeBytes = 123456,
                               .m_filePath = "test.rom",
                               .m_fileMd5 = "d41d8cd98f00b204e9800998ecf8427e",
                               .m_fileCrc32 = "12345678",
                               .m_inArchive = false,
                               .m_archivePathName = "",
                               .m_platformId = 1,
                               .m_contentHash = "d41d8cd98f00b204e9800998ecf8427e"};

  ASSERT_TRUE(library.create(romInfo));

  auto entry = library.getEntryWithContentHash(romInfo.m_contentHash);
  ASSERT_TRUE(entry.has_value());
  ASSERT_TRUE(entry->isContentAvailable);

  auto actualEntry = library.getEntryWithContentHash(romInfo.m_contentHash);
  ASSERT_EQ(entry->id, actualEntry->id);

  ASSERT_TRUE(library.deleteContentFile(romInfo.m_id));

  // Get entry again after deleting one rom
  actualEntry = library.getEntryWithContentHash(romInfo.m_contentHash);
  ASSERT_TRUE(actualEntry.has_value());
  ASSERT_FALSE(actualEntry->isContentAvailable);
  ASSERT_FALSE(actualEntry->hidden) << "losing a file put the entry away on the user's behalf";
}

TEST_F(SqliteUserLibraryTest, AddingRomAfterDeletingMakesEntryAvailableAgainTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::DiscSetService discSets(library, "");
  library::LibraryIngestService ingest(library, discSets);

  library::ContentFile romInfo{.m_fileSizeBytes = 123456,
                               .m_filePath = "test.rom",
                               .m_fileMd5 = "d41d8cd98f00b204e9800998ecf8427e",
                               .m_fileCrc32 = "12345678",
                               .m_inArchive = false,
                               .m_archivePathName = "",
                               .m_platformId = 1,
                               .m_contentHash = "d41d8cd98f00b204e9800998ecf8427e"};

  ASSERT_TRUE(library.create(romInfo));

  auto entry = library.getEntryWithContentHash(romInfo.m_contentHash);
  ASSERT_TRUE(entry.has_value());
  ASSERT_TRUE(entry->isContentAvailable);

  auto actualEntry = library.getEntryWithContentHash(romInfo.m_contentHash);
  ASSERT_EQ(entry->id, actualEntry->id);

  ASSERT_TRUE(library.deleteContentFile(romInfo.m_id));

  actualEntry = library.getEntryWithContentHash(romInfo.m_contentHash);
  ASSERT_TRUE(actualEntry.has_value());
  ASSERT_FALSE(actualEntry->isContentAvailable);

  ASSERT_TRUE(library.create(romInfo));

  entry = library.getEntryWithContentHash(romInfo.m_contentHash);
  ASSERT_TRUE(entry.has_value());
  ASSERT_TRUE(entry->isContentAvailable);
}

TEST_F(SqliteUserLibraryTest, RomsMarkedMissingWhenContentDirectoryDeletedTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::DiscSetService discSets(library, "");
  library::LibraryIngestService ingest(library, discSets);

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
                               .m_contentHash = "d41d8cd98f00b204e9800998ecf8427e"};

  ASSERT_TRUE(library.create(romInfo));

  // Verify the rom file was added
  auto actualRomInfo = library.getContentFile(romInfo.m_id);
  ASSERT_TRUE(actualRomInfo.has_value());

  // Delete the content directory
  ASSERT_TRUE(library.deleteContentDirectory(main.id));

  // The row outlives the folder, so the game can still say where it was
  actualRomInfo = library.getContentFile(romInfo.m_id);
  ASSERT_TRUE(actualRomInfo.has_value());
  EXPECT_NE(actualRomInfo->m_missingSince, 0);
  EXPECT_EQ(actualRomInfo->m_filePath, "test_content_directory/test.rom");
  EXPECT_TRUE(library.getPresentContentFiles().empty()) << "a file under a removed folder still counts as content";

  const auto entry = library.getEntryWithContentHash("d41d8cd98f00b204e9800998ecf8427e");
  ASSERT_TRUE(entry.has_value());
  EXPECT_FALSE(entry->isContentAvailable) << "the game is unlaunchable but still on the shelf";
}

// TODO: Delete content directory, deletes all rom files in it, does the above

TEST_F(SqliteUserLibraryTest, UpdateEntryTest) {
  // TODO
}

TEST_F(SqliteUserLibraryTest, SmartFolderTypeAndFilterJsonRoundTripTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");

  auto info = library::FolderInfo{.displayName = "SNES favorites",
                                  .type = static_cast<int>(library::FolderType::Smart),
                                  .filterJson = R"({"platformIds":[3],"favorite":true})"};
  ASSERT_TRUE(library.create(info));

  auto folders = library.listFolders();
  ASSERT_EQ(folders.size(), 1);
  ASSERT_EQ(folders[0].type, static_cast<int>(library::FolderType::Smart));
  ASSERT_EQ(folders[0].filterJson, R"({"platformIds":[3],"favorite":true})");

  // A plain folder defaults to Manual with no criteria
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

  // Updating criteria persists
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
  library::DiscSetService discSets(library, "");
  library::LibraryIngestService ingest(library, discSets);

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

  // A file under no known directory gets -1
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
  library::DiscSetService discSets(library, "");
  library::LibraryIngestService ingest(library, discSets);

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

TEST_F(SqliteUserLibraryTest, EntryFileLocationsPopulatedTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::DiscSetService discSets(library, "");
  library::LibraryIngestService ingest(library, discSets);

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
// FolderInfo (id -1) if absent
library::FolderInfo findFolder(const std::vector<library::FolderInfo> &folders, int id) {
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

  auto info = library::FolderInfo{
      .displayName = "styled", .color = "#ff8800", .sortRole = "lastPlayedAt", .sortAscending = false};
  ASSERT_TRUE(library.create(info));

  auto stored = findFolder(library.listFolders(), info.id);
  ASSERT_EQ(stored.color, "#ff8800");
  ASSERT_EQ(stored.sortRole, "lastPlayedAt");
  ASSERT_FALSE(stored.sortAscending);

  // Update changes appearance
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

  // listFolders returns them in position order
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

  // New order: c, a, b (parent -1 = root)
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

  // Move both under parent; each appends at the end of parent's scope
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

TEST_F(SqliteUserLibraryTest, EntryFileLocationsUseArchivePathTest) {
  auto library = library::SqliteUserLibraryRepository(":memory:");
  library::DiscSetService discSets(library, "");
  library::LibraryIngestService ingest(library, discSets);

  library::ContentDirectory dir{.path = "roms"};
  ASSERT_TRUE(library.create(dir));

  // An archived entry's on-disk location is the archive path; the file location
  // resolves against that, and contentPaths reports it (so "path contains"
  // works for archived content too)
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

// Variant groups: a set of entries that are the same game. Membership lives on the
// entry, so removing a group has to clear its members by hand -- there are no
// foreign keys anywhere in this schema
// TODO
// The whole-library load and the single-entry read fill in the same per-file facts by different
// SQL, so they are pinned against each other: an entry read either way says the same thing
TEST_F(SqliteUserLibraryTest, BothLoadersAgreeOnAnEntrysFiles) {
  library::SqliteUserLibraryRepository library(":memory:");

  library::ContentDirectory dir;
  dir.path = "roms";
  ASSERT_TRUE(library.create(dir));

  const auto addDump = [&](const std::string &path, const int discNumber, const bool isMissing) {
    library::ContentFile file;
    file.m_type = library::ContentType::Disc;
    file.m_filePath = path;
    file.m_fileSizeBytes = 1;
    file.m_fileMd5 = path;
    file.m_platformId = 6;
    file.m_contentHash = "sharedHash";
    file.m_discNumber = discNumber;
    file.m_contentDirectoryId = dir.id;
    ASSERT_TRUE(library.create(file));

    if (isMissing) {
      ASSERT_TRUE(library.markContentFileMissing(file.m_id));
    }
  };

  // One numbered, one not, and one that has gone away: the three cases the ordering has to settle
  addDump("roms/game-d2.cue", 2, false);
  addDump("roms/game.cue", 0, false);
  addDump("roms/game-d1.cue", 1, true);

  library::Entry first{.displayName = "Game", .contentHash = "sharedHash", .platformId = 6};
  ASSERT_TRUE(library.createEntry(first));

  // TODO
  // One entry per hash, which is what makes an entry a facade over its ways in rather than one of
  // them. The index does not say so, so this is the only thing that does
  library::Entry second{.displayName = "Game (again)", .contentHash = "sharedHash", .platformId = 6};
  EXPECT_FALSE(library.createEntry(second)) << "a second entry took the same hash";

  const auto all = library.getEntries();
  ASSERT_EQ(all.size(), 1u);

  for (const auto &grouped : all) {
    const auto single = library.getEntry(grouped.id);
    ASSERT_TRUE(single.has_value());

    EXPECT_EQ(grouped.contentPaths, single->contentPaths) << "the two loaders disagree on where the content is";
    EXPECT_EQ(grouped.readableContentPaths, single->readableContentPaths);
    EXPECT_EQ(grouped.contentDirectoryIds, single->contentDirectoryIds);
    EXPECT_EQ(grouped.isContentAvailable, single->isContentAvailable);
    EXPECT_EQ(grouped.isDiscInArchive, single->isDiscInArchive);
    EXPECT_EQ(grouped.hasRunConfiguration, single->hasRunConfiguration);

    // Every dump on the hash, not just the first one found
    EXPECT_EQ(grouped.contentPaths.size(), 3u);

    // A file carrying no number sorts behind the ones that are, so disc 1 leads the paths
    EXPECT_EQ(grouped.contentPaths.front(), "roms/game-d1.cue");

    // The disc that went missing is still somewhere the content lives, just not readable
    EXPECT_EQ(grouped.readableContentPaths.size(), 2u);
  }
}

// A multi-disc game is one set holding several content files. The membership edge is on the
// file, so the set is found from its discs rather than from the entry
TEST_F(SqliteUserLibraryTest, DiscSetHoldsItsDiscsInOrder) {
  library::SqliteUserLibraryRepository library(":memory:");

  library::DiscSet set{.title = "Final Fantasy VII"};
  ASSERT_TRUE(library.createDiscSet(set));
  EXPECT_GT(set.id, 0);

  const auto makeDisc = [&](const std::string &path, const int discNumber) {
    library::ContentFile file;
    file.m_filePath = path;
    file.m_fileSizeBytes = 1;
    file.m_fileMd5 = path;
    file.m_fileCrc32 = "";
    file.m_platformId = 6;
    file.m_contentHash = path;
    file.m_discNumber = discNumber;
    library.create(file);

    library::DiscSetMember member{
        .m_discSetId = set.id, .m_discNumber = discNumber, .m_contentFileId = file.m_id, .m_memberPath = path};
    library.create(member);
    return file.m_id;
  };

  // Added out of order to prove the ordering comes from the disc number, not insertion
  makeDisc("ff7-d3.cue", 3);
  const auto discOneId = makeDisc("ff7-d1.cue", 1);
  makeDisc("ff7-d2.cue", 2);

  const auto discs = library.getDiscsInSet(set.id);
  ASSERT_EQ(discs.size(), 3u);
  EXPECT_EQ(discs[0].m_discNumber, 1);
  EXPECT_EQ(discs[1].m_discNumber, 2);
  EXPECT_EQ(discs[2].m_discNumber, 3);

  const auto found = library.getDiscSetForContentFile(discOneId);
  ASSERT_TRUE(found.has_value());
  EXPECT_EQ(found->title, "Final Fantasy VII");
}

// A disc kept as both a cue and a chd is two rows of identical bytes. Counting it twice reads
// as a set with more discs than the game has, which badges a complete game as missing one
TEST_F(SqliteUserLibraryTest, ADiscDumpedTwiceCountsOnce) {
  library::SqliteUserLibraryRepository library(":memory:");

  library::DiscSet set{.title = "Final Fantasy VII"};
  ASSERT_TRUE(library.createDiscSet(set));

  const auto makeDisc = [&](const std::string &path, const int discNumber, const std::string &contentHash) {
    library::ContentFile file;
    file.m_filePath = path;
    file.m_fileSizeBytes = 1;
    file.m_fileMd5 = path;
    file.m_fileCrc32 = "";
    file.m_platformId = 6;
    file.m_contentHash = contentHash;
    file.m_discNumber = discNumber;
    library.create(file);

    library::DiscSetMember member{
        .m_discSetId = set.id, .m_discNumber = discNumber, .m_contentFileId = file.m_id, .m_memberPath = path};
    library.create(member);
  };

  makeDisc("ff7-d1.cue", 1, "hash-disc-1");
  makeDisc("ff7-d1.chd", 1, "hash-disc-1");
  makeDisc("ff7-d2.cue", 2, "hash-disc-2");

  const auto discs = library.getDiscsInSet(set.id);
  ASSERT_EQ(discs.size(), 2u);
  EXPECT_EQ(discs[0].m_discNumber, 1);
  EXPECT_EQ(discs[1].m_discNumber, 2);
}

// Folding a set deletes the entries of every disc but one, so what a dump is has to live on
// the file rather than on the entry that is about to go
TEST_F(SqliteUserLibraryTest, PerDumpFactsRoundTrip) {
  library::SqliteUserLibraryRepository library(":memory:");

  library::ContentFile file;
  file.m_filePath = "ff7-d1.cue";
  file.m_fileSizeBytes = 1;
  file.m_fileMd5 = "md5";
  file.m_fileCrc32 = "";
  file.m_platformId = 6;
  file.m_contentHash = "hash-disc-1";
  file.m_discNumber = 1;
  file.m_regions = {"US", "EU"};
  ASSERT_TRUE(library.create(file));

  const auto stored = library.getContentFile(file.m_id);
  ASSERT_TRUE(stored.has_value());
  EXPECT_EQ(stored->m_regions, (std::vector<std::string>{"US", "EU"}));
}

// TODO
// The query is a net, not an answer: a title can be carried by the entry or only by the dumps
// behind it, so it returns everything either could reach and the predicate decides
TEST_F(SqliteUserLibraryTest, CandidateEntriesAreEverythingEitherKeyCouldReach) {
  library::SqliteUserLibraryRepository library(":memory:");

  const auto add = [&](const std::string &hash, const std::string &entryTitle, const std::string &dumpTitle) {
    library::ContentFile file;
    file.m_filePath = hash + ".cue";
    file.m_fileSizeBytes = 1;
    file.m_fileMd5 = hash;
    file.m_fileCrc32 = "";
    file.m_platformId = 7;
    file.m_contentHash = hash;
    file.m_normalizedTitle = dumpTitle;
    library.create(file);

    library::Entry entry{.displayName = entryTitle, .contentHash = hash, .platformId = 7};
    entry.normalizedTitle = entryTitle;
    library.createEntry(entry);
    library.updateEntryMetadata(entry);
    return entry.id;
  };

  const auto byBoth = add("h1", "resident evil 2", "resident evil 2");
  const auto byDumpOnly = add("h2", "", "resident evil 2");
  const auto byEntryOnly = add("h3", "resident evil 2", "");
  add("h4", "grandia", "grandia");

  // Both arms, merged, ascending and without repeats
  const auto found = library.getCandidateEntryIds(library::GameIdentity{.platformId = 7, .title = "resident evil 2"});
  EXPECT_EQ(found, (std::vector{byBoth, byDumpOnly, byEntryOnly}));

  // Nothing known reaches nothing, rather than reaching everything
  EXPECT_TRUE(library.getCandidateEntryIds(library::GameIdentity{.platformId = 7}).empty());

  // Another platform's game is never a candidate
  EXPECT_TRUE(library.getCandidateEntryIds(library::GameIdentity{.platformId = 8, .title = "resident evil 2"}).empty());
}

// A file that got past the extension gate and could not be catalogued is kept per path, because
// the answer to "where did my game go" is a path
TEST_F(SqliteUserLibraryTest, ScanDropsAreRecordedPerPathAndClearedWhenTheyIdentify) {
  library::SqliteUserLibraryRepository library(":memory:");

  const library::ScanDrop drop{.filePath = "C:/roms/Game.cso",
                               .extension = "cso",
                               .fileSizeBytes = 4096,
                               .outcome = library::IdentifyOutcome::NoIdentifier};

  EXPECT_TRUE(library.recordScanDrop(drop)) << "the first sighting should read as new";
  EXPECT_FALSE(library.recordScanDrop(drop)) << "a rescan of the same file is not a new drop";

  const auto drops = library.getScanDrops();
  ASSERT_EQ(drops.size(), 1u) << "recording the same path twice made two rows";
  EXPECT_EQ(drops.front().filePath, "C:/roms/Game.cso");
  EXPECT_EQ(drops.front().extension, "cso");
  EXPECT_EQ(drops.front().outcome, library::IdentifyOutcome::NoIdentifier);
  EXPECT_EQ(drops.front().fileSizeBytes, 4096u);

  // The table holds only what is still wrong, so identifying later takes the row away
  EXPECT_TRUE(library.clearScanDrop("C:/roms/Game.cso", ""));
  EXPECT_TRUE(library.getScanDrops().empty());
}

// One file inside an archive and one loose at the same entry name are different files
TEST_F(SqliteUserLibraryTest, ADropInsideAnArchiveIsItsOwnRow) {
  library::SqliteUserLibraryRepository library(":memory:");

  ASSERT_TRUE(library.recordScanDrop({.filePath = "Game.cso", .extension = "cso"}));
  ASSERT_TRUE(library.recordScanDrop({.filePath = "Game.cso", .archivePath = "C:/roms/a.zip", .extension = "cso"}));

  EXPECT_EQ(library.getScanDrops().size(), 2u);
}

// A count and never a path: a folder of three thousand save files has to be one row
TEST_F(SqliteUserLibraryTest, UnrecognizedExtensionsAreCountedNotListed) {
  library::SqliteUserLibraryRepository library(":memory:");

  for (auto i = 0; i < 3; ++i) {
    library.countUnrecognizedExtension("srm");
  }
  library.countUnrecognizedExtension("fds");

  const auto extensions = library.getUnrecognizedExtensions();
  ASSERT_EQ(extensions.size(), 2u);
  // Most common first, so the field report reads as a priority list
  EXPECT_EQ(extensions[0].extension, "srm");
  EXPECT_EQ(extensions[0].count, 3);
  EXPECT_EQ(extensions[1].extension, "fds");
  EXPECT_EQ(extensions[1].count, 1);
}

// Nothing having said where a dump is from is not the same as it being from nowhere
TEST_F(SqliteUserLibraryTest, AnUnstampedRegionReadsBackEmpty) {
  library::SqliteUserLibraryRepository library(":memory:");

  library::ContentFile file;
  file.m_filePath = "game.sfc";
  file.m_fileSizeBytes = 1;
  file.m_fileMd5 = "md5";
  file.m_fileCrc32 = "";
  file.m_platformId = 6;
  file.m_contentHash = "hash";
  ASSERT_TRUE(library.create(file));

  const auto stored = library.getContentFile(file.m_id);
  ASSERT_TRUE(stored.has_value());
  EXPECT_TRUE(stored->m_regions.empty());
}

// Dissolving a set must not take the discs or the entry with it
TEST_F(SqliteUserLibraryTest, DeletingADiscSetKeepsItsDiscsAndEntry) {
  library::SqliteUserLibraryRepository library(":memory:");

  library::DiscSet set{.title = "Zelda"};
  ASSERT_TRUE(library.createDiscSet(set));

  library::ContentFile file;
  file.m_filePath = "zelda-d1.cue";
  file.m_fileSizeBytes = 1;
  file.m_fileMd5 = "zelda-d1";
  file.m_fileCrc32 = "";
  file.m_platformId = 6;
  file.m_contentHash = "zeldaHash";
  ASSERT_TRUE(library.create(file));

  library::DiscSetMember member{
      .m_discSetId = set.id, .m_discNumber = 1, .m_contentFileId = file.m_id, .m_memberPath = file.m_filePath};
  ASSERT_TRUE(library.create(member));

  library::Entry entry;
  entry.displayName = "Zelda";
  entry.contentHash = "zeldaHash";
  entry.platformId = 6;
  ASSERT_TRUE(library.createEntry(entry));
  ASSERT_TRUE(library.deleteDiscSet(set.id));

  EXPECT_FALSE(library.getDiscSet(set.id).has_value());
  EXPECT_TRUE(library.getEntry(entry.id).has_value());
  EXPECT_TRUE(library.getEntriesInDiscSet(set.id).empty()) << "the set still reaches an entry";
  EXPECT_FALSE(library.getDiscSetForContentFile(file.m_id).has_value());
}

// Two playthroughs of one game sit on different discs, so the last disc is remembered per
// save slot rather than per game
TEST_F(SqliteUserLibraryTest, LastDiscIsRememberedPerSaveSlot) {
  library::SqliteUserLibraryRepository library(":memory:");

  library::Entry entry;
  entry.displayName = "Grandia";
  entry.contentHash = "grandiaHash";
  entry.platformId = 6;
  ASSERT_TRUE(library.createEntry(entry));

  EXPECT_FALSE(library.getLastDisc(entry.id, 1).has_value());

  ASSERT_TRUE(library.setLastDisc(entry.id, 1, 2));
  ASSERT_TRUE(library.setLastDisc(entry.id, 2, 1));

  EXPECT_EQ(library.getLastDisc(entry.id, 1), 2);
  EXPECT_EQ(library.getLastDisc(entry.id, 2), 1);

  // Writing the same slot again moves it rather than adding a second row
  ASSERT_TRUE(library.setLastDisc(entry.id, 1, 3));
  EXPECT_EQ(library.getLastDisc(entry.id, 1), 3);
  EXPECT_EQ(library.getLastDisc(entry.id, 2), 1);
}

TEST_F(SqliteUserLibraryTest, VariantGroupRoundTrips) {
  auto library = library::SqliteUserLibraryRepository(":memory:");

  library::VariantGroup group{.title = "Chrono Trigger"};
  ASSERT_TRUE(library.createVariantGroup(group));
  ASSERT_NE(group.id, -1);

  const auto stored = library.getVariantGroup(group.id);
  ASSERT_TRUE(stored.has_value());
  EXPECT_EQ(stored->title, "Chrono Trigger");
  EXPECT_FALSE(stored->primaryEntryId.has_value());
  EXPECT_FALSE(stored->primaryUserSet);
}

TEST_F(SqliteUserLibraryTest, EntriesJoinAndLeaveAVariantGroup) {
  auto library = library::SqliteUserLibraryRepository(":memory:");

  library::VariantGroup group{.title = "Super Metroid"};
  ASSERT_TRUE(library.createVariantGroup(group));

  library::Entry usa{.displayName = "Super Metroid (USA)", .contentHash = "hashUsa", .platformId = 6};
  library::Entry jp{.displayName = "Super Metroid (Japan)", .contentHash = "hashJp", .platformId = 6};
  ASSERT_TRUE(library.createEntry(usa));
  ASSERT_TRUE(library.createEntry(jp));

  ASSERT_TRUE(library.setEntryVariantGroup(usa.id, group.id, true));
  ASSERT_TRUE(library.setEntryVariantGroup(jp.id, group.id, true));
  EXPECT_EQ(library.getEntriesInVariantGroup(group.id).size(), 2u);

  ASSERT_TRUE(library.setEntryVariantGroup(jp.id, std::nullopt, true));
  EXPECT_EQ(library.getEntriesInVariantGroup(group.id).size(), 1u);

  const auto loose = library.getEntry(jp.id);
  ASSERT_TRUE(loose.has_value());
  EXPECT_FALSE(loose->variantGroupId.has_value());
}

TEST_F(SqliteUserLibraryTest, DeletingAVariantGroupReleasesItsMembers) {
  auto library = library::SqliteUserLibraryRepository(":memory:");

  library::VariantGroup group{.title = "Zelda"};
  ASSERT_TRUE(library.createVariantGroup(group));

  library::Entry entry{.displayName = "Zelda (USA)", .contentHash = "hashZelda", .platformId = 5};
  ASSERT_TRUE(library.createEntry(entry));
  ASSERT_TRUE(library.setEntryVariantGroup(entry.id, group.id, true));

  ASSERT_TRUE(library.deleteVariantGroup(group.id));

  EXPECT_FALSE(library.getVariantGroup(group.id).has_value());

  const auto released = library.getEntry(entry.id);
  ASSERT_TRUE(released.has_value());
  EXPECT_FALSE(released->variantGroupId.has_value());
}

// The whole point of the override set: a scrape must leave a user's typing alone
TEST_F(SqliteUserLibraryTest, ApplyEntryMetadataRespectsTheOverrideSet) {
  auto library = library::SqliteUserLibraryRepository(":memory:");

  library::Entry entry{.displayName = "game.sfc", .contentHash = "hashMeta", .platformId = 6};
  ASSERT_TRUE(library.createEntry(entry));

  GameMetadata mine;
  mine.description = "My own words";
  ASSERT_TRUE(library.applyEntryMetadata(entry.id, mine, {metadata_fields::DESCRIPTION}, true));

  GameMetadata scraped;
  scraped.description = "Scraped words";
  scraped.developer = "Nintendo";
  ASSERT_TRUE(
      library.applyEntryMetadata(entry.id, scraped, {metadata_fields::DESCRIPTION, metadata_fields::DEVELOPER}, false));

  const auto stored = library.getEntry(entry.id);
  ASSERT_TRUE(stored.has_value());
  EXPECT_EQ(stored->metadata.description, "My own words");
  EXPECT_EQ(stored->metadata.developer, "Nintendo");
  EXPECT_TRUE(stored->metadataOverrides.isUserSet(metadata_fields::DESCRIPTION));
  EXPECT_FALSE(stored->metadataOverrides.isUserSet(metadata_fields::DEVELOPER));
}

// A field only becomes pinned when the change is the user's; a scrape writing it
// must not make it immune to the next scrape
TEST_F(SqliteUserLibraryTest, OnlyAUserEditPinsAField) {
  auto library = library::SqliteUserLibraryRepository(":memory:");

  library::Entry entry{.displayName = "game.sfc", .contentHash = "hashPin", .platformId = 6};
  ASSERT_TRUE(library.createEntry(entry));

  GameMetadata first;
  first.developer = "Old";
  ASSERT_TRUE(library.applyEntryMetadata(entry.id, first, {metadata_fields::DEVELOPER}, false));

  GameMetadata second;
  second.developer = "New";
  ASSERT_TRUE(library.applyEntryMetadata(entry.id, second, {metadata_fields::DEVELOPER}, false));

  const auto stored = library.getEntry(entry.id);
  ASSERT_TRUE(stored.has_value());
  EXPECT_EQ(stored->metadata.developer, "New");
  EXPECT_TRUE(stored->metadataOverrides.fields.empty());
}

// Tags are the user's own vocabulary, so one spelling per idea is enforceable here
// in a way it is not for scraped genres
TEST_F(SqliteUserLibraryTest, TagNamesCollideCaseInsensitively) {
  auto library = library::SqliteUserLibraryRepository(":memory:");

  library::Tag lower{.name = "sci-fi"};
  library::Tag upper{.name = "Sci-Fi"};

  ASSERT_TRUE(library.createTag(lower));
  ASSERT_TRUE(library.createTag(upper));

  EXPECT_EQ(lower.id, upper.id);
  EXPECT_EQ(library.getTags().size(), 1u);
}

TEST_F(SqliteUserLibraryTest, SetEntryTagsReplacesAndCounts) {
  auto library = library::SqliteUserLibraryRepository(":memory:");

  library::Tag favourite{.name = "favourite"};
  library::Tag backlog{.name = "backlog"};
  ASSERT_TRUE(library.createTag(favourite));
  ASSERT_TRUE(library.createTag(backlog));

  library::Entry entry{.displayName = "game.sfc", .contentHash = "hashTag", .platformId = 6};
  ASSERT_TRUE(library.createEntry(entry));

  ASSERT_TRUE(library.setEntryTags(entry.id, {favourite.id, backlog.id}));

  auto stored = library.getEntry(entry.id);
  ASSERT_TRUE(stored.has_value());
  EXPECT_EQ(stored->tagIds.size(), 2u);

  // Setting is a replace, not an add
  ASSERT_TRUE(library.setEntryTags(entry.id, {backlog.id}));

  stored = library.getEntry(entry.id);
  ASSERT_TRUE(stored.has_value());
  EXPECT_EQ(stored->tagIds, (std::vector<int>{backlog.id}));

  for (const auto &tag : library.getTags()) {
    EXPECT_EQ(tag.usageCount, tag.id == backlog.id ? 1 : 0);
  }
}

TEST_F(SqliteUserLibraryTest, RenamingATagKeepsItsEntries) {
  auto library = library::SqliteUserLibraryRepository(":memory:");

  library::Tag tag{.name = "rpg"};
  ASSERT_TRUE(library.createTag(tag));

  library::Entry entry{.displayName = "game.sfc", .contentHash = "hashRename", .platformId = 6};
  ASSERT_TRUE(library.createEntry(entry));
  ASSERT_TRUE(library.setEntryTags(entry.id, {tag.id}));

  ASSERT_TRUE(library.renameTag(tag.id, "Role-Playing"));

  const auto tags = library.getTags();
  ASSERT_EQ(tags.size(), 1u);
  EXPECT_EQ(tags.front().name, "Role-Playing");
  EXPECT_EQ(tags.front().usageCount, 1);
}

// The case the OR IGNORE exists for: an entry carrying both tags would otherwise
// trip UNIQUE(entry_id, tag_id) and fail the whole merge
TEST_F(SqliteUserLibraryTest, MergingTagsAbsorbsEntriesCarryingBoth) {
  auto library = library::SqliteUserLibraryRepository(":memory:");

  library::Tag source{.name = "scifi"};
  library::Tag target{.name = "science-fiction"};
  ASSERT_TRUE(library.createTag(source));
  ASSERT_TRUE(library.createTag(target));

  library::Entry both{.displayName = "both.sfc", .contentHash = "hashBoth", .platformId = 6};
  library::Entry onlySource{.displayName = "one.sfc", .contentHash = "hashOne", .platformId = 6};
  ASSERT_TRUE(library.createEntry(both));
  ASSERT_TRUE(library.createEntry(onlySource));

  ASSERT_TRUE(library.setEntryTags(both.id, {source.id, target.id}));
  ASSERT_TRUE(library.setEntryTags(onlySource.id, {source.id}));

  ASSERT_TRUE(library.mergeTags(source.id, target.id));

  const auto tags = library.getTags();
  ASSERT_EQ(tags.size(), 1u);
  EXPECT_EQ(tags.front().id, target.id);
  EXPECT_EQ(tags.front().usageCount, 2);

  const auto stored = library.getEntry(both.id);
  ASSERT_TRUE(stored.has_value());
  EXPECT_EQ(stored->tagIds, (std::vector<int>{target.id}));
}

TEST_F(SqliteUserLibraryTest, DeletingATagTakesItOffEveryEntry) {
  auto library = library::SqliteUserLibraryRepository(":memory:");

  library::Tag tag{.name = "temporary"};
  ASSERT_TRUE(library.createTag(tag));

  library::Entry entry{.displayName = "game.sfc", .contentHash = "hashDelete", .platformId = 6};
  ASSERT_TRUE(library.createEntry(entry));
  ASSERT_TRUE(library.setEntryTags(entry.id, {tag.id}));

  ASSERT_TRUE(library.deleteTag(tag.id));

  EXPECT_TRUE(library.getTags().empty());

  const auto stored = library.getEntry(entry.id);
  ASSERT_TRUE(stored.has_value());
  EXPECT_TRUE(stored->tagIds.empty());
}

// getEntries loads folders, tags and file locations in grouped queries rather than
// per entry, so it has to attach each to the right row
TEST_F(SqliteUserLibraryTest, GetEntriesAttachesJoinsToTheRightRows) {
  auto library = library::SqliteUserLibraryRepository(":memory:");

  library::Tag tag{.name = "rpg"};
  ASSERT_TRUE(library.createTag(tag));
  auto folder = library::FolderInfo{.displayName = "Favourites"};
  ASSERT_TRUE(library.create(folder));

  library::Entry tagged{.displayName = "Alpha", .contentHash = "hashAlpha", .platformId = 6};
  library::Entry bare{.displayName = "Bravo", .contentHash = "hashBravo", .platformId = 6};
  ASSERT_TRUE(library.createEntry(tagged));
  ASSERT_TRUE(library.createEntry(bare));

  ASSERT_TRUE(library.setEntryTags(tagged.id, {tag.id}));
  auto membership = library::FolderEntry{.folderId = folder.id, .entryId = tagged.id};
  ASSERT_TRUE(library.create(membership));

  const auto entries = library.getEntries();
  ASSERT_EQ(entries.size(), 2u);

  for (const auto &entry : entries) {
    if (entry.id == tagged.id) {
      EXPECT_EQ(entry.tagIds, (std::vector<int>{tag.id}));
      EXPECT_EQ(entry.folderIds, (std::vector<int>{folder.id}));
    } else {
      EXPECT_TRUE(entry.tagIds.empty());
      EXPECT_TRUE(entry.folderIds.empty());
    }
  }
}

// TODO
// A copy sitting in a zip says nothing while another sits loose on disk. This flag refuses the
// launch outright, so reading it as "any copy is archived" hides a game whose files are right here
TEST_F(SqliteUserLibraryTest, AnArchivedCopyDoesNotCondemnAGameWhoseLooseCopyIsHere) {
  library::SqliteUserLibraryRepository library(":memory:");

  const auto addDisc = [&](const std::string &path, const bool inArchive) -> int {
    library::ContentFile file;
    file.m_type = library::ContentType::Disc;
    file.m_filePath = path;
    file.m_fileSizeBytes = 1;
    file.m_fileMd5 = path;
    file.m_platformId = 7;
    file.m_contentHash = "oneDump";
    file.m_inArchive = inArchive;
    file.m_archivePathName = inArchive ? "roms/backup.zip" : "";
    EXPECT_TRUE(library.create(file));
    return file.m_id;
  };

  addDisc("roms/Game (Disc 1).chd", false);
  const auto archived = addDisc("Game (Disc 1).chd", true);

  library::Entry entry{.displayName = "Game", .contentHash = "oneDump", .platformId = 7};
  ASSERT_TRUE(library.createEntry(entry));

  const auto both = library.getEntry(entry.id);
  ASSERT_TRUE(both.has_value());
  EXPECT_TRUE(both->isContentAvailable);
  EXPECT_FALSE(both->isDiscInArchive) << "a zipped duplicate condemned a game whose loose copy is here";

  // With the loose copy gone, the only way to reach the game really is through the archive
  ASSERT_TRUE(library.markContentFileMissing(library.getContentFileWithPath("roms/Game (Disc 1).chd")->m_id));

  const auto onlyArchived = library.getEntry(entry.id);
  ASSERT_TRUE(onlyArchived.has_value());
  EXPECT_TRUE(onlyArchived->isContentAvailable);
  EXPECT_TRUE(onlyArchived->isDiscInArchive) << "the only readable copy being zipped went unreported";

  // And with nothing readable at all, the archive is not the problem worth reporting
  ASSERT_TRUE(library.markContentFileMissing(archived));

  const auto none = library.getEntry(entry.id);
  ASSERT_TRUE(none.has_value());
  EXPECT_FALSE(none->isContentAvailable);
  EXPECT_FALSE(none->isDiscInArchive) << "a game with nothing on disk reported an archive problem on top";
}

} // namespace firelight::db

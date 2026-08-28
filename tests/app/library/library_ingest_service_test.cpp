// TODO: NEEDS REVIEW
#include <firelight/event_dispatcher.hpp>
#include <firelight/library/library_events.hpp>
#include <firelight/library/library_ingest_service.hpp>
#include <firelight/library/sqlite_user_library.hpp>

#include <gtest/gtest.h>

namespace firelight::library {

// Exercises the scan-time orchestration end to end: the ingest service subscribes
// to the repository's domain events (published through the global EventDispatcher)
// and turns discovered content into visible/hidden library entries. Each test gets
// a fresh in-memory repository + ingest service, so the subscriptions are torn
// down with the fixture
class LibraryIngestServiceTest : public testing::Test {
protected:
  SqliteUserLibraryRepository m_repo{":memory:"};
  LibraryIngestService m_ingest{m_repo};

  static ContentFile makeContentFile(const std::string &path, int platformId, const std::string &hash) {
    ContentFile cf;
    cf.m_type = ContentType::Cartridge;
    cf.m_filePath = path;
    cf.m_platformId = platformId;
    cf.m_contentHash = hash;
    cf.m_fileMd5 = "md5-" + hash;
    cf.m_fileCrc32 = "crc-" + hash;
    return cf;
  }

  static int countEntriesWithHash(const std::vector<Entry> &entries, const std::string &hash) {
    int n = 0;
    for (const auto &e : entries) {
      if (e.contentHash == hash) {
        ++n;
      }
    }
    return n;
  }
};

// The realistic chain: adding a content file publishes ContentFileAddedEvent,
// which the ingest service turns into a run configuration and, in turn, a
// visible entry whose display name is the file's basename
TEST_F(LibraryIngestServiceTest, AddingContentFileCreatesVisibleEntry) {
  auto cf = makeContentFile("/roms/gba/Cool Game.gba", 3, "hashA");
  ASSERT_TRUE(m_repo.create(cf));

  const auto entry = m_repo.getEntryWithContentHash("hashA");
  ASSERT_TRUE(entry.has_value());
  EXPECT_EQ(entry->displayName, "Cool Game.gba");
  EXPECT_EQ(entry->platformId, 3u);
  EXPECT_FALSE(entry->hidden);
  EXPECT_FALSE(m_repo.getRunConfigurations("hashA").empty());
}

// A second way in for content that already has an entry attaches to it rather than standing up
// another one
TEST_F(LibraryIngestServiceTest, RunConfigForExistingContentDoesNotDuplicateTheEntry) {
  auto cf = makeContentFile("/roms/Game.gba", 3, "hashB");
  ASSERT_TRUE(m_repo.create(cf));

  EventDispatcher::instance().publish(
      RunConfigurationCreatedEvent{.id = 999, .filePath = "/roms/Game.gba", .platformId = 3, .contentHash = "hashB"});

  EXPECT_EQ(countEntriesWithHash(m_repo.getEntries(), "hashB"), 1);
}

// Availability follows the files, not the ways in, so it is the file being marked missing that
// takes a game off the shelf — and hidden stays the user's to set
TEST_F(LibraryIngestServiceTest, AvailabilityFollowsTheFileRatherThanTheRunConfiguration) {
  auto cf = makeContentFile("/roms/Solo.gba", 3, "hashC");
  ASSERT_TRUE(m_repo.create(cf));

  auto entry = m_repo.getEntryWithContentHash("hashC");
  ASSERT_TRUE(entry.has_value());
  EXPECT_TRUE(entry->isContentAvailable);
  EXPECT_FALSE(entry->hidden);

  ASSERT_TRUE(m_repo.markContentFileMissing(cf.m_id));

  entry = m_repo.getEntryWithContentHash("hashC");
  ASSERT_TRUE(entry.has_value());
  EXPECT_FALSE(entry->isContentAvailable);
  EXPECT_FALSE(entry->hidden) << "availability wrote the field the user is supposed to own";

  ASSERT_TRUE(m_repo.reviveContentFile(cf.m_id));

  entry = m_repo.getEntryWithContentHash("hashC");
  ASSERT_TRUE(entry.has_value());
  EXPECT_TRUE(entry->isContentAvailable);
}

// The reason removal hides rather than deletes: everything the user built on top of
// an entry has to be there when the file comes back. content_hash is what makes the
// returning file land on the same row rather than a new one
TEST_F(LibraryIngestServiceTest, RemovingAndReaddingAFileKeepsTheEntryIntact) {
  auto contentFile = makeContentFile("/roms/Chrono Trigger (USA).sfc", 6, "hashKeep");
  ASSERT_TRUE(m_repo.create(contentFile));

  auto entry = m_repo.getEntryWithContentHash("hashKeep");
  ASSERT_TRUE(entry.has_value());
  const auto originalId = entry->id;

  // Everything a user can put on an entry
  entry->displayName = "Chrono Trigger";
  entry->nameUserSet = true;
  entry->favorite = true;
  entry->rating = 5;
  entry->activeSaveSlot = 3;
  ASSERT_TRUE(m_repo.update(*entry));

  GameMetadata edited;
  edited.description = "My own words";
  ASSERT_TRUE(m_repo.applyEntryMetadata(originalId, edited, {metadata_fields::DESCRIPTION}, true));

  Tag tag{.name = "finished"};
  ASSERT_TRUE(m_repo.createTag(tag));
  ASSERT_TRUE(m_repo.setEntryTags(originalId, {tag.id}));

  auto folder = FolderInfo{.displayName = "Favourites"};
  ASSERT_TRUE(m_repo.create(folder));
  auto membership = FolderEntry{.folderId = folder.id, .entryId = originalId};
  ASSERT_TRUE(m_repo.create(membership));

  VariantGroup group{.title = "Chrono Trigger"};
  ASSERT_TRUE(m_repo.createVariantGroup(group));
  ASSERT_TRUE(m_repo.setEntryVariantGroup(originalId, group.id, true));

  // The file goes away
  ASSERT_TRUE(m_repo.deleteContentFile(contentFile.m_id));

  entry = m_repo.getEntryWithContentHash("hashKeep");
  ASSERT_TRUE(entry.has_value());
  EXPECT_FALSE(entry->isContentAvailable);

  // ...and comes back
  auto readded = makeContentFile("D:/elsewhere/Chrono Trigger (USA).sfc", 6, "hashKeep");
  ASSERT_TRUE(m_repo.create(readded));

  entry = m_repo.getEntryWithContentHash("hashKeep");
  ASSERT_TRUE(entry.has_value());
  EXPECT_TRUE(entry->isContentAvailable);

  EXPECT_EQ(entry->id, originalId);
  EXPECT_EQ(entry->displayName, "Chrono Trigger");
  EXPECT_TRUE(entry->nameUserSet);
  EXPECT_TRUE(entry->favorite);
  EXPECT_EQ(entry->rating, 5u);
  EXPECT_EQ(entry->activeSaveSlot, 3u);
  EXPECT_EQ(entry->metadata.description, "My own words");
  EXPECT_TRUE(entry->metadataOverrides.isUserSet(metadata_fields::DESCRIPTION));
  EXPECT_EQ(entry->tagIds, (std::vector<int>{tag.id}));
  EXPECT_EQ(entry->folderIds, (std::vector<int>{folder.id}));
  EXPECT_EQ(entry->variantGroupId, group.id);

  // Only one row for the content, so the re-add did not duplicate it
  EXPECT_EQ(countEntriesWithHash(m_repo.getEntries(), "hashKeep"), 1);
}

// A delete event while other run configurations still exist must NOT hide the
// entry (only the removal of the *last* one hides it)
TEST_F(LibraryIngestServiceTest, DeleteEventKeepsEntryVisibleWhileRunConfigsRemain) {
  auto cf = makeContentFile("/roms/Game.gba", 3, "hashD");
  ASSERT_TRUE(m_repo.create(cf));
  ASSERT_FALSE(m_repo.getRunConfigurations("hashD").empty());

  EventDispatcher::instance().publish(RunConfigurationDeletedEvent{.contentHash = "hashD"});

  const auto entry = m_repo.getEntryWithContentHash("hashD");
  ASSERT_TRUE(entry.has_value());
  EXPECT_FALSE(entry->hidden);
}

// A file at the filesystem root (no '/') uses the whole path as the display name
// (guards the basename extraction's npos branch)
TEST_F(LibraryIngestServiceTest, DisplayNameFallsBackToWholePathWithoutSlash) {
  EventDispatcher::instance().publish(
      RunConfigurationCreatedEvent{.id = 1, .filePath = "Game.gba", .platformId = 3, .contentHash = "hashE"});
  const auto entry = m_repo.getEntryWithContentHash("hashE");
  ASSERT_TRUE(entry.has_value());
  EXPECT_EQ(entry->displayName, "Game.gba");
}

} // namespace firelight::library

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

  static ContentFile makeContentFile(const std::string &path, int platformId,
                                     const std::string &hash) {
    ContentFile cf;
    cf.m_type = ContentType::Cartridge;
    cf.m_filePath = path;
    cf.m_platformId = platformId;
    cf.m_contentHash = hash;
    cf.m_fileMd5 = "md5-" + hash;
    cf.m_fileCrc32 = "crc-" + hash;
    return cf;
  }

  static int countEntriesWithHash(const std::vector<Entry> &entries,
                                   const std::string &hash) {
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

// A run configuration for content whose entry already exists (and is hidden)
// unhides it instead of creating a duplicate
TEST_F(LibraryIngestServiceTest, RunConfigForHiddenEntryUnhidesWithoutDuplicating) {
  auto cf = makeContentFile("/roms/Game.gba", 3, "hashB");
  ASSERT_TRUE(m_repo.create(cf));

  auto entry = m_repo.getEntryWithContentHash("hashB");
  ASSERT_TRUE(entry.has_value());
  entry->hidden = true;
  ASSERT_TRUE(m_repo.update(*entry));

  // A second run configuration for the same content hash arrives
  EventDispatcher::instance().publish(
      RunConfigurationCreatedEvent{.id = 999,
                                   .filePath = "/roms/Game.gba",
                                   .platformId = 3,
                                   .contentHash = "hashB"});

  const auto refreshed = m_repo.getEntryWithContentHash("hashB");
  ASSERT_TRUE(refreshed.has_value());
  EXPECT_FALSE(refreshed->hidden);
  // No duplicate entry was created for the same content
  EXPECT_EQ(countEntriesWithHash(m_repo.getEntries(0, -1), "hashB"), 1);
}

// When the last run configuration for a content hash is removed, its entry is
// hidden (it disappears from the library without losing user state)
TEST_F(LibraryIngestServiceTest, RemovingLastRunConfigHidesEntry) {
  // Create an entry via a run-config event without a backing run_configurations
  // row, so getRunConfigurations() is empty for this hash
  EventDispatcher::instance().publish(
      RunConfigurationCreatedEvent{.id = 1,
                                   .filePath = "/roms/Solo.gba",
                                   .platformId = 3,
                                   .contentHash = "hashC"});
  auto entry = m_repo.getEntryWithContentHash("hashC");
  ASSERT_TRUE(entry.has_value());
  ASSERT_FALSE(entry->hidden);

  EventDispatcher::instance().publish(
      RunConfigurationDeletedEvent{.contentHash = "hashC"});

  entry = m_repo.getEntryWithContentHash("hashC");
  ASSERT_TRUE(entry.has_value());
  EXPECT_TRUE(entry->hidden);
}

// A delete event while other run configurations still exist must NOT hide the
// entry (only the removal of the *last* one hides it)
TEST_F(LibraryIngestServiceTest, DeleteEventKeepsEntryVisibleWhileRunConfigsRemain) {
  auto cf = makeContentFile("/roms/Game.gba", 3, "hashD");
  ASSERT_TRUE(m_repo.create(cf));
  ASSERT_FALSE(m_repo.getRunConfigurations("hashD").empty());

  EventDispatcher::instance().publish(
      RunConfigurationDeletedEvent{.contentHash = "hashD"});

  const auto entry = m_repo.getEntryWithContentHash("hashD");
  ASSERT_TRUE(entry.has_value());
  EXPECT_FALSE(entry->hidden);
}

// A file at the filesystem root (no '/') uses the whole path as the display name
// (guards the basename extraction's npos branch)
TEST_F(LibraryIngestServiceTest, DisplayNameFallsBackToWholePathWithoutSlash) {
  EventDispatcher::instance().publish(
      RunConfigurationCreatedEvent{.id = 1,
                                   .filePath = "Game.gba",
                                   .platformId = 3,
                                   .contentHash = "hashE"});
  const auto entry = m_repo.getEntryWithContentHash("hashE");
  ASSERT_TRUE(entry.has_value());
  EXPECT_EQ(entry->displayName, "Game.gba");
}

} // namespace firelight::library

#include <firelight/library/entry_resolver.hpp>
#include <firelight/library/library_ingest_service.hpp>
#include <firelight/library/sqlite_user_library.hpp>

#include <gtest/gtest.h>

namespace firelight::library {

// Adding a content file creates its run configuration and entry; resolving that
// entry must return the content file (no patch)
TEST(EntryResolverTest, ResolvesRomEntry) {
  SqliteUserLibraryRepository library(":memory:");
  LibraryIngestService ingest(library);

  ContentFile romInfo{.m_fileSizeBytes = 100,
                      .m_filePath = "game.rom",
                      .m_fileMd5 = "abc",
                      .m_fileCrc32 = "123",
                      .m_inArchive = false,
                      .m_archivePathName = "",
                      .m_platformId = 1,
                      .m_contentHash = "hash123"};
  ASSERT_TRUE(library.create(romInfo));

  const auto entry = library.getEntryWithContentHash("hash123");
  ASSERT_TRUE(entry.has_value());

  EntryResolver resolver(library);
  const auto resolved = resolver.resolve(*entry);

  ASSERT_TRUE(resolved.valid);
  ASSERT_EQ(resolved.contentFile.m_id, romInfo.m_id);
  ASSERT_EQ(resolved.contentFile.m_contentHash, "hash123");
  ASSERT_FALSE(resolved.patch.has_value());
}

// An entry with no run configurations cannot be resolved
TEST(EntryResolverTest, InvalidWhenNoRunConfigurations) {
  SqliteUserLibraryRepository library(":memory:");

  EntryResolver resolver(library);
  Entry entry;
  entry.contentHash = "does-not-exist";

  ASSERT_FALSE(resolver.resolve(entry).valid);
}

} // namespace firelight::library

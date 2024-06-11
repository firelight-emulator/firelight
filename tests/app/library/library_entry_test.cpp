#include "firelight/library_entry.hpp"
#include <gtest/gtest.h>

namespace firelight::db {
class LibraryEntryTest : public testing::Test {};

TEST_F(LibraryEntryTest, DefaultValuesTest) {
  const LibraryEntry entry;

  ASSERT_EQ(entry.id, -1);
  ASSERT_EQ(entry.displayName, "");
  ASSERT_EQ(entry.contentId, "");
  ASSERT_EQ(entry.platformId, -1);
  ASSERT_EQ(entry.parentEntryId, -1);
  ASSERT_EQ(entry.activeSaveSlot, 1);
  ASSERT_EQ(entry.type, LibraryEntry::EntryType::UNKNOWN);
  ASSERT_EQ(entry.fileMd5, "");
  ASSERT_EQ(entry.fileCrc32, "");
  ASSERT_EQ(entry.sourceDirectory, "");
  ASSERT_EQ(entry.contentPath, "");
  ASSERT_EQ(entry.createdAt, 0);
}

} // namespace firelight::db

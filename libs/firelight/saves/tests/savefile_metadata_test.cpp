#include <firelight/saves/savefile_metadata.hpp>

#include <gtest/gtest.h>

namespace firelight::saves {

TEST(SavefileMetadataTest, DefaultValuesTest) {
  const SavefileMetadata metadata;

  ASSERT_EQ(metadata.id, -1);
  ASSERT_EQ(metadata.contentId, "");
  ASSERT_EQ(metadata.slotNumber, 1u);
  ASSERT_EQ(metadata.savefileMd5, "");
  ASSERT_EQ(metadata.lastModifiedAt, 0);
  ASSERT_EQ(metadata.createdAt, 0);
}

} // namespace firelight::saves

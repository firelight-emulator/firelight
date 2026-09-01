// TODO: NEEDS REVIEW
#include <firelight/library/metadata_overrides.hpp>
#include <firelight/util/game_metadata.hpp>

#include <gtest/gtest.h>

// Verifies the stored metadata document and the set naming which of its fields a user pinned
namespace firelight::library {

TEST(MetadataOverridesTest, RoundTripsAndAnswersIsUserSet) {
  MetadataOverrides overrides;
  overrides.markUserSet(metadata_fields::DESCRIPTION);
  overrides.markUserSet(metadata_fields::GENRES);

  const auto restored = MetadataOverrides::parse(overrides.toJson());

  EXPECT_TRUE(restored.isUserSet(metadata_fields::DESCRIPTION));
  EXPECT_TRUE(restored.isUserSet(metadata_fields::GENRES));
  EXPECT_FALSE(restored.isUserSet(metadata_fields::DEVELOPER));
}

TEST(MetadataOverridesTest, MarkingTwiceIsIdempotent) {
  MetadataOverrides overrides;
  overrides.markUserSet(metadata_fields::PUBLISHER);
  overrides.markUserSet(metadata_fields::PUBLISHER);

  EXPECT_EQ(overrides.fields.size(), 1u);

  overrides.clearUserSet(metadata_fields::PUBLISHER);

  EXPECT_FALSE(overrides.isUserSet(metadata_fields::PUBLISHER));
  EXPECT_TRUE(overrides.fields.empty());
}

TEST(MetadataOverridesTest, MalformedInputYieldsAnEmptySet) {
  EXPECT_TRUE(MetadataOverrides::parse("").fields.empty());
  EXPECT_TRUE(MetadataOverrides::parse("{\"description\":true}").fields.empty());
  EXPECT_TRUE(MetadataOverrides::parse("garbage").fields.empty());
}

} // namespace firelight::library

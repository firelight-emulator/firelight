#include <firelight/util/game_metadata.hpp>

#include <gtest/gtest.h>

// Verifies the stored metadata document and the set naming which of its fields a user pinned
namespace firelight {

TEST(EntryMetadataTest, RoundTrips) {
  GameMetadata original;
  original.description = "A role playing game";
  original.developer = "Squaresoft";
  original.publisher = "Square";
  original.releaseYear = 1995;
  original.releaseDate = "1995-03-11";
  original.players = "1";
  original.revision = "1";
  original.genres = {"RPG", "Adventure"};
  original.regions = {"JP", "US"};
  original.languages = {"ja", "en"};
  original.flags = {"verified-dump"};

  const auto restored = GameMetadata::parse(original.toJson());

  EXPECT_EQ(restored.description, original.description);
  EXPECT_EQ(restored.developer, original.developer);
  EXPECT_EQ(restored.publisher, original.publisher);
  EXPECT_EQ(restored.releaseYear, original.releaseYear);
  EXPECT_EQ(restored.releaseDate, original.releaseDate);
  EXPECT_EQ(restored.players, original.players);
  EXPECT_EQ(restored.revision, original.revision);
  EXPECT_EQ(restored.genres, original.genres);
  EXPECT_EQ(restored.regions, original.regions);
  EXPECT_EQ(restored.languages, original.languages);
  EXPECT_EQ(restored.flags, original.flags);
}

TEST(EntryMetadataTest, OmitsEmptyFields) {
  GameMetadata metadata;
  metadata.developer = "Nintendo";

  const auto json = metadata.toJson();

  EXPECT_NE(json.find("developer"), std::string::npos);
  EXPECT_EQ(json.find("description"), std::string::npos);
  EXPECT_EQ(json.find("releaseYear"), std::string::npos);
  EXPECT_EQ(json.find("genres"), std::string::npos);
}

// Same contract as SmartFolderCriteria: bad input degrades rather than throwing, because it is read
// on the path that loads the whole library
TEST(EntryMetadataTest, MalformedInputYieldsAnEmptyValue) {
  EXPECT_TRUE(GameMetadata::parse("").isEmpty());
  EXPECT_TRUE(GameMetadata::parse("not json at all").isEmpty());
  EXPECT_TRUE(GameMetadata::parse("[1,2,3]").isEmpty());
  EXPECT_TRUE(GameMetadata::parse("{").isEmpty());
}

TEST(EntryMetadataTest, IgnoresUnknownKeysAndWrongTypes) {
  const auto metadata = GameMetadata::parse(R"({"developer":"Sega","somethingElse":42,"genres":"not an array"})");

  EXPECT_EQ(metadata.developer, "Sega");
  EXPECT_TRUE(metadata.genres.empty());
}

} // namespace firelight

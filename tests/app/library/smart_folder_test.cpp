#include <firelight/library/smart_folder.hpp>

#include <gtest/gtest.h>

namespace firelight::library {
namespace {

// A fully-populated entry the tests narrow down from. platformId 3 (SNES-ish),
// favorite, RPG, developer/publisher Squaresoft, 1997, in content dirs {1,2},
// paths under .../roms/snes/, played recently for an hour.
EntryFields sampleEntry() {
  EntryFields e;
  e.platformId = 3;
  e.favorite = true;
  e.genres = "Role-Playing, Adventure";
  e.developer = "Squaresoft";
  e.publisher = "Squaresoft";
  e.releaseYear = 1997;
  e.contentDirectoryIds = {1, 2};
  e.contentPaths = {"C:/roms/snes/final_fantasy.sfc"};
  e.lastPlayedMillis = 2000;
  e.secondsPlayed = 3600;
  return e;
}

TEST(SmartFolderTest, EmptyCriteriaMatchesEverything) {
  SmartFolderCriteria c;
  EXPECT_TRUE(c.isEmpty());
  EXPECT_TRUE(matches(sampleEntry(), c));
  EXPECT_TRUE(matches(EntryFields{}, c));
}

TEST(SmartFolderTest, PlatformFilterOrWithinList) {
  SmartFolderCriteria c;
  c.platformIds = {5, 3};
  EXPECT_TRUE(matches(sampleEntry(), c));

  c.platformIds = {5, 6};
  EXPECT_FALSE(matches(sampleEntry(), c));
}

TEST(SmartFolderTest, FavoriteFilter) {
  auto e = sampleEntry();
  SmartFolderCriteria c;
  c.favorite = true;
  EXPECT_TRUE(matches(e, c));

  c.favorite = false;
  EXPECT_FALSE(matches(e, c));

  e.favorite = false;
  EXPECT_TRUE(matches(e, c));
}

TEST(SmartFolderTest, GenreMatchesAnyCaseInsensitive) {
  SmartFolderCriteria c;
  c.genres = {"role-playing"};
  EXPECT_TRUE(matches(sampleEntry(), c));

  c.genres = {"Shooter", "adventure"}; // any-of; "adventure" present
  EXPECT_TRUE(matches(sampleEntry(), c));

  c.genres = {"Shooter", "Puzzle"};
  EXPECT_FALSE(matches(sampleEntry(), c));
}

TEST(SmartFolderTest, DeveloperAndPublisherSubstring) {
  SmartFolderCriteria c;
  c.developer = "square";
  EXPECT_TRUE(matches(sampleEntry(), c));

  c.developer = "nintendo";
  EXPECT_FALSE(matches(sampleEntry(), c));

  SmartFolderCriteria c2;
  c2.publisher = "SQUARE";
  EXPECT_TRUE(matches(sampleEntry(), c2));
}

TEST(SmartFolderTest, YearRange) {
  SmartFolderCriteria c;
  c.yearMin = 1990;
  c.yearMax = 1999;
  EXPECT_TRUE(matches(sampleEntry(), c));

  c.yearMin = 1998;
  EXPECT_FALSE(matches(sampleEntry(), c));

  SmartFolderCriteria c2;
  c2.yearMax = 1996;
  EXPECT_FALSE(matches(sampleEntry(), c2));
}

TEST(SmartFolderTest, UnknownYearExcludedFromYearFilter) {
  auto e = sampleEntry();
  e.releaseYear = 0; // unknown

  SmartFolderCriteria c;
  c.yearMin = 1990;
  EXPECT_FALSE(matches(e, c));

  SmartFolderCriteria c2;
  c2.yearMax = 2000;
  EXPECT_FALSE(matches(e, c2));
}

TEST(SmartFolderTest, PlayHistory) {
  auto e = sampleEntry();

  SmartFolderCriteria c;
  c.playedAfterMillis = 1000;
  EXPECT_TRUE(matches(e, c));
  c.playedAfterMillis = 5000;
  EXPECT_FALSE(matches(e, c));

  SmartFolderCriteria c2;
  c2.minSecondsPlayed = 3600;
  EXPECT_TRUE(matches(e, c2));
  c2.minSecondsPlayed = 3601;
  EXPECT_FALSE(matches(e, c2));

  // Never-played entry fails a "played since" filter.
  EntryFields never;
  SmartFolderCriteria c3;
  c3.playedAfterMillis = 1;
  EXPECT_FALSE(matches(never, c3));
}

TEST(SmartFolderTest, SourceContentDirectoryIntersection) {
  SmartFolderCriteria c;
  c.contentDirectoryIds = {2, 9}; // entry is in {1,2} -> intersects on 2
  EXPECT_TRUE(matches(sampleEntry(), c));

  c.contentDirectoryIds = {7, 9};
  EXPECT_FALSE(matches(sampleEntry(), c));
}

TEST(SmartFolderTest, SourcePathContainsCaseInsensitive) {
  SmartFolderCriteria c;
  c.pathContains = "SNES";
  EXPECT_TRUE(matches(sampleEntry(), c));

  c.pathContains = "genesis";
  EXPECT_FALSE(matches(sampleEntry(), c));
}

TEST(SmartFolderTest, CriteriaAndTogether) {
  SmartFolderCriteria c;
  c.platformIds = {3};
  c.favorite = true;
  c.pathContains = "snes";
  EXPECT_TRUE(matches(sampleEntry(), c));

  // One failing criterion (favorite) fails the whole match.
  auto e = sampleEntry();
  e.favorite = false;
  EXPECT_FALSE(matches(e, c));
}

TEST(SmartFolderTest, JsonRoundTrip) {
  SmartFolderCriteria c;
  c.contentDirectoryIds = {1, 2};
  c.pathContains = "snes";
  c.platformIds = {3, 4};
  c.favorite = true;
  c.genres = {"RPG"};
  c.developer = "Squaresoft";
  c.publisher = "Nintendo";
  c.yearMin = 1990;
  c.yearMax = 1999;
  c.playedAfterMillis = 123456789;
  c.minSecondsPlayed = 3600;

  const auto parsed = SmartFolderCriteria::parse(c.toJson());
  EXPECT_EQ(parsed.contentDirectoryIds, c.contentDirectoryIds);
  EXPECT_EQ(parsed.pathContains, c.pathContains);
  EXPECT_EQ(parsed.platformIds, c.platformIds);
  EXPECT_EQ(parsed.favorite, c.favorite);
  EXPECT_EQ(parsed.genres, c.genres);
  EXPECT_EQ(parsed.developer, c.developer);
  EXPECT_EQ(parsed.publisher, c.publisher);
  EXPECT_EQ(parsed.yearMin, c.yearMin);
  EXPECT_EQ(parsed.yearMax, c.yearMax);
  EXPECT_EQ(parsed.playedAfterMillis, c.playedAfterMillis);
  EXPECT_EQ(parsed.minSecondsPlayed, c.minSecondsPlayed);
}

TEST(SmartFolderTest, EmptyJsonAndMalformedJsonAreMatchAll) {
  EXPECT_TRUE(SmartFolderCriteria::parse("").isEmpty());
  EXPECT_TRUE(SmartFolderCriteria::parse("not json{{{").isEmpty());
  EXPECT_TRUE(SmartFolderCriteria::parse("[]").isEmpty());
  EXPECT_TRUE(SmartFolderCriteria::parse("{}").isEmpty());
}

TEST(SmartFolderTest, ToJsonOmitsUnsetFields) {
  SmartFolderCriteria c;
  c.favorite = true;
  const auto json = c.toJson();
  // Only "favorite" should be present.
  EXPECT_NE(json.find("favorite"), std::string::npos);
  EXPECT_EQ(json.find("pathContains"), std::string::npos);
  EXPECT_EQ(json.find("platformIds"), std::string::npos);
}

} // namespace
} // namespace firelight::library

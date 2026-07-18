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
  EXPECT_TRUE(c.matches(sampleEntry()));
  EXPECT_TRUE(c.matches(EntryFields{}));
}

TEST(SmartFolderTest, PlatformFilterOrWithinList) {
  SmartFolderCriteria c;
  c.platformIds = {5, 3};
  EXPECT_TRUE(c.matches(sampleEntry()));

  c.platformIds = {5, 6};
  EXPECT_FALSE(c.matches(sampleEntry()));
}

TEST(SmartFolderTest, FavoriteFilter) {
  auto e = sampleEntry();
  SmartFolderCriteria c;
  c.favorite = true;
  EXPECT_TRUE(c.matches(e));

  c.favorite = false;
  EXPECT_FALSE(c.matches(e));

  e.favorite = false;
  EXPECT_TRUE(c.matches(e));
}

TEST(SmartFolderTest, GenreMatchesAnyCaseInsensitive) {
  SmartFolderCriteria c;
  c.genres = {"role-playing"};
  EXPECT_TRUE(c.matches(sampleEntry()));

  c.genres = {"Shooter", "adventure"}; // any-of; "adventure" present
  EXPECT_TRUE(c.matches(sampleEntry()));

  c.genres = {"Shooter", "Puzzle"};
  EXPECT_FALSE(c.matches(sampleEntry()));
}

TEST(SmartFolderTest, DeveloperAndPublisherSubstring) {
  SmartFolderCriteria c;
  c.developer = "square";
  EXPECT_TRUE(c.matches(sampleEntry()));

  c.developer = "nintendo";
  EXPECT_FALSE(c.matches(sampleEntry()));

  SmartFolderCriteria c2;
  c2.publisher = "SQUARE";
  EXPECT_TRUE(c2.matches(sampleEntry()));
}

TEST(SmartFolderTest, YearRange) {
  SmartFolderCriteria c;
  c.yearMin = 1990;
  c.yearMax = 1999;
  EXPECT_TRUE(c.matches(sampleEntry()));

  c.yearMin = 1998;
  EXPECT_FALSE(c.matches(sampleEntry()));

  SmartFolderCriteria c2;
  c2.yearMax = 1996;
  EXPECT_FALSE(c2.matches(sampleEntry()));
}

TEST(SmartFolderTest, UnknownYearExcludedFromYearFilter) {
  auto e = sampleEntry();
  e.releaseYear = 0; // unknown

  SmartFolderCriteria c;
  c.yearMin = 1990;
  EXPECT_FALSE(c.matches(e));

  SmartFolderCriteria c2;
  c2.yearMax = 2000;
  EXPECT_FALSE(c2.matches(e));
}

TEST(SmartFolderTest, PlayHistory) {
  auto e = sampleEntry();

  SmartFolderCriteria c;
  c.playedAfterMillis = 1000;
  EXPECT_TRUE(c.matches(e));
  c.playedAfterMillis = 5000;
  EXPECT_FALSE(c.matches(e));

  SmartFolderCriteria c2;
  c2.minSecondsPlayed = 3600;
  EXPECT_TRUE(c2.matches(e));
  c2.minSecondsPlayed = 3601;
  EXPECT_FALSE(c2.matches(e));

  // Never-played entry fails a "played since" filter.
  EntryFields never;
  SmartFolderCriteria c3;
  c3.playedAfterMillis = 1;
  EXPECT_FALSE(c3.matches(never));
}

TEST(SmartFolderTest, SourceContentDirectoryIntersection) {
  SmartFolderCriteria c;
  c.contentDirectoryIds = {2, 9}; // entry is in {1,2} -> intersects on 2
  EXPECT_TRUE(c.matches(sampleEntry()));

  c.contentDirectoryIds = {7, 9};
  EXPECT_FALSE(c.matches(sampleEntry()));
}

TEST(SmartFolderTest, SourcePathContainsCaseInsensitive) {
  SmartFolderCriteria c;
  c.pathContains = "SNES";
  EXPECT_TRUE(c.matches(sampleEntry()));

  c.pathContains = "genesis";
  EXPECT_FALSE(c.matches(sampleEntry()));
}

TEST(SmartFolderTest, CriteriaAndTogether) {
  SmartFolderCriteria c;
  c.platformIds = {3};
  c.favorite = true;
  c.pathContains = "snes";
  EXPECT_TRUE(c.matches(sampleEntry()));

  // One failing criterion (favorite) fails the whole match.
  auto e = sampleEntry();
  e.favorite = false;
  EXPECT_FALSE(c.matches(e));
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
  c.playedWithinDays = 14;
  c.unplayed = false;

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
  EXPECT_EQ(parsed.playedWithinDays, c.playedWithinDays);
  EXPECT_EQ(parsed.unplayed, c.unplayed);
}

TEST(SmartFolderTest, UnplayedCriterion) {
  auto played = sampleEntry(); // lastPlayedMillis = 2000
  auto never = sampleEntry();
  never.lastPlayedMillis = 0;

  SmartFolderCriteria unplayedOnly;
  unplayedOnly.unplayed = true;
  EXPECT_TRUE(unplayedOnly.matches(never, 10000));
  EXPECT_FALSE(unplayedOnly.matches(played, 10000));

  SmartFolderCriteria playedOnly;
  playedOnly.unplayed = false;
  EXPECT_TRUE(playedOnly.matches(played, 10000));
  EXPECT_FALSE(playedOnly.matches(never, 10000));
}

TEST(SmartFolderTest, PlayedWithinDaysRollingWindow) {
  const int64_t day = 86400000LL;
  const int64_t now = 100 * day;

  SmartFolderCriteria c;
  c.playedWithinDays = 7;

  auto recent = sampleEntry();
  recent.lastPlayedMillis = now - 3 * day; // within the window
  EXPECT_TRUE(c.matches(recent, now));

  auto stale = sampleEntry();
  stale.lastPlayedMillis = now - 10 * day; // outside the window
  EXPECT_FALSE(c.matches(stale, now));

  auto never = sampleEntry();
  never.lastPlayedMillis = 0; // never played is excluded from any window
  EXPECT_FALSE(c.matches(never, now));
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

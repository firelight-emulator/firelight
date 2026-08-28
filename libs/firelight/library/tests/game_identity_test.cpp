#include <firelight/library/game_identity.hpp>

#include <gtest/gtest.h>

// One rule, applied to every field: it says the same thing, a different thing, or nothing at
// all -- and nothing at all never counts as different
namespace firelight::library {

namespace {
GameIdentity identity(const int gameId, const std::string &title, const std::vector<std::string> &regions = {},
                      const int discNumber = 0, const unsigned platformId = 7) {
  return GameIdentity{
      .platformId = platformId, .gameId = gameId, .title = title, .regions = regions, .discNumber = discNumber};
}
} // namespace

// Before a content database exists every id is 0, so the folded title is the whole key
TEST(GameIdentityTest, WithoutIdsTheTitleDecides) {
  EXPECT_TRUE(areSameGame(identity(0, "final fantasy vii"), identity(0, "final fantasy vii")));
  EXPECT_FALSE(areSameGame(identity(0, "final fantasy vii"), identity(0, "final fantasy viii")));
}

// An entry nothing has named is not the same game as every other entry nothing has named
TEST(GameIdentityTest, TwoUnknownsAreNotEachOther) {
  EXPECT_FALSE(areSameGame(identity(0, ""), identity(0, "")));
  EXPECT_TRUE(identity(0, "").isEmpty());
  EXPECT_FALSE(identity(0, "grandia").isEmpty());
  EXPECT_FALSE(identity(25390, "").isEmpty());
}

// What the database is for: one game whose releases are called different things
TEST(GameIdentityTest, AResolvedIdBeatsDifferentTitles) {
  EXPECT_TRUE(areSameGame(identity(25390, "biohazard 2"), identity(25390, "resident evil 2")));
}

// And the other direction: two games that happen to share a name
TEST(GameIdentityTest, DifferentIdsAreDifferentGamesHoweverTheyAreNamed) {
  EXPECT_FALSE(areSameGame(identity(11, "gauntlet"), identity(22, "gauntlet")));
}

// The case that would split a disc set if an id were treated as strict: a database that knows
// one dump and not the other still has to leave them together
TEST(GameIdentityTest, PartialCoverageFallsBackToTheTitle) {
  EXPECT_TRUE(areSameGame(identity(25390, "resident evil 2"), identity(0, "resident evil 2")));
  EXPECT_TRUE(areSameGame(identity(0, "resident evil 2"), identity(25390, "resident evil 2")));
  EXPECT_FALSE(areSameGame(identity(25390, "resident evil 2"), identity(0, "resident evil 3")));
}

TEST(GameIdentityTest, OnePlatformsGameIsNotAnothers) {
  EXPECT_FALSE(areSameGame(identity(0, "resident evil 2", {}, 0, 7), identity(0, "resident evil 2", {}, 0, 8)));
  EXPECT_FALSE(areSameGame(identity(25390, "resident evil 2", {}, 0, 7), identity(25390, "resident evil 2", {}, 0, 8)));
}

// Nothing having said where a dump is from is not the same as it being from somewhere else
TEST(GameIdentityTest, AnUnknownRegionIsCompatibleWithEverything) {
  EXPECT_TRUE(areRegionsCompatible({}, {"US"}));
  EXPECT_TRUE(areRegionsCompatible({"US"}, {}));
  EXPECT_TRUE(areRegionsCompatible({}, {}));
  EXPECT_TRUE(areRegionsCompatible({"US"}, {"US"}));
  EXPECT_FALSE(areRegionsCompatible({"US"}, {"JP"}));
}

// Deliberate, and pinned so it is a decision rather than an accident: the order is part of the
// value, because it is stored most authoritative first
TEST(GameIdentityTest, RegionOrderIsPartOfTheValue) {
  EXPECT_FALSE(areRegionsCompatible({"US", "EU"}, {"EU", "US"}));
}

// Variants span releases and pin the disc axis: a set's discs must not read as versions of
// each other, or all but one disappears behind a primary
TEST(GameIdentityTest, VariantsAreTheSameGameStandingInTheSamePlace) {
  EXPECT_TRUE(areVariants(identity(0, "grandia", {"US"}, 1), identity(0, "grandia", {"JP"}, 1)));
  EXPECT_TRUE(areVariants(identity(0, "grandia"), identity(0, "grandia")));
  EXPECT_FALSE(areVariants(identity(0, "grandia", {}, 1), identity(0, "grandia", {}, 2)));
  EXPECT_FALSE(areVariants(identity(11, "gauntlet"), identity(22, "gauntlet")));
}

// Disc sets span discs and pin the release axis, which is the mirror image
TEST(GameIdentityTest, DiscsOfOneReleaseSpanDiscNumbersButNotRegions) {
  EXPECT_TRUE(areDiscsOfOneRelease(identity(0, "grandia", {"US"}, 1), identity(0, "grandia", {"US"}, 2)));
  EXPECT_TRUE(areDiscsOfOneRelease(identity(0, "grandia", {}, 1), identity(0, "grandia", {"US"}, 2)));
  EXPECT_FALSE(areDiscsOfOneRelease(identity(0, "grandia", {"US"}, 1), identity(0, "grandia", {"JP"}, 2)));
}

// The Resident Evil 2 pair, both ways. One game id spans Japan and the USA, so it is the payoff
// for variants and the hazard for disc sets -- a JP disc 1 and a US disc 2 are the same game and
// are not each other's discs
TEST(GameIdentityTest, OneGameIdAcrossRegionsGroupsVariantsAndSeparatesDiscs) {
  const auto japan = identity(25390, "biohazard 2", {"JP"}, 1);
  const auto usa = identity(25390, "resident evil 2", {"US"}, 1);

  EXPECT_TRUE(areVariants(japan, usa));
  EXPECT_FALSE(areDiscsOfOneRelease(japan, identity(25390, "resident evil 2", {"US"}, 2)));
}

// Two copies of one disc have to reach the grouper so it can refuse to guess which release
// each came from, rather than being filtered out of sight here
TEST(GameIdentityTest, RepeatedDiscNumbersStillReadAsOneRelease) {
  EXPECT_TRUE(areDiscsOfOneRelease(identity(0, "parasite eve", {}, 2), identity(0, "parasite eve", {}, 2)));
}

// A field added to the key without being projected would read as zero and quietly stop mattering
TEST(GameIdentityTest, IdentityIsReadFromTheEntryFieldByField) {
  Entry entry;
  entry.platformId = 7;
  entry.normalizedTitle = "resident evil 2";
  entry.metadata.regions = {"US"};
  entry.discNumber = 2;

  const auto identity = identityOf(entry);

  EXPECT_EQ(identity.platformId, 7u);
  EXPECT_EQ(identity.gameId, 25390);
  EXPECT_EQ(identity.title, "resident evil 2");
  EXPECT_EQ(identity.regions, (std::vector<std::string>{"US"}));
  EXPECT_EQ(identity.discNumber, 2);
}

} // namespace firelight::library

// TODO: NEEDS REVIEW
#include <firelight/library/game_identity.hpp>

#include <gtest/gtest.h>

// One rule, applied to every field: it says the same thing, a different thing, or nothing at
// all -- and nothing at all never counts as different
namespace firelight::library {

namespace {
GameIdentity identity(const std::string &title, const std::vector<std::string> &regions = {}, const int discNumber = 0,
                      const unsigned platformId = 7) {
  return GameIdentity{.platformId = platformId, .title = title, .regions = regions, .discNumber = discNumber};
}
} // namespace

// TODO
// The folded title is the whole key. There is no id to fall back to and there will not be one:
// the content database's id spans regions and discs, so it could never decide either question
TEST(GameIdentityTest, TheTitleDecides) {
  EXPECT_TRUE(areSameGame(identity("final fantasy vii"), identity("final fantasy vii")));
  EXPECT_FALSE(areSameGame(identity("final fantasy vii"), identity("final fantasy viii")));
}

// An entry nothing has named is not the same game as every other entry nothing has named
TEST(GameIdentityTest, TwoUnknownsAreNotEachOther) {
  EXPECT_FALSE(areSameGame(identity(""), identity("")));
  EXPECT_TRUE(identity("").isEmpty());
  EXPECT_FALSE(identity("grandia").isEmpty());
}

TEST(GameIdentityTest, OnePlatformsGameIsNotAnothers) {
  EXPECT_FALSE(areSameGame(identity("resident evil 2", {}, 0, 7), identity("resident evil 2", {}, 0, 8)));
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
TEST(GameIdentityTest, RegionOrderIsPartOfTheValue) { EXPECT_FALSE(areRegionsCompatible({"US", "EU"}, {"EU", "US"})); }

// Variants span releases and pin the disc axis: a set's discs must not read as versions of
// each other, or all but one disappears behind a primary
TEST(GameIdentityTest, VariantsAreTheSameGameStandingInTheSamePlace) {
  EXPECT_TRUE(areVariants(identity("grandia", {"US"}, 1), identity("grandia", {"JP"}, 1)));
  EXPECT_TRUE(areVariants(identity("grandia"), identity("grandia")));
  EXPECT_FALSE(areVariants(identity("grandia", {}, 1), identity("grandia", {}, 2)));
  EXPECT_FALSE(areVariants(identity("gauntlet"), identity("gauntlet ii")));
}

// Disc sets span discs and pin the release axis, which is the mirror image
TEST(GameIdentityTest, DiscsOfOneReleaseSpanDiscNumbersButNotRegions) {
  EXPECT_TRUE(areDiscsOfOneRelease(identity("grandia", {"US"}, 1), identity("grandia", {"US"}, 2)));
  EXPECT_TRUE(areDiscsOfOneRelease(identity("grandia", {}, 1), identity("grandia", {"US"}, 2)));
  EXPECT_FALSE(areDiscsOfOneRelease(identity("grandia", {"US"}, 1), identity("grandia", {"JP"}, 2)));
}

// TODO
// The Resident Evil 2 pair, both ways. One title across Japan and the USA is the payoff for
// variants and the hazard for disc sets: a JP disc 1 and a US disc 2 are the same game and are
// not each other's discs
TEST(GameIdentityTest, OneTitleAcrossRegionsGroupsVariantsAndSeparatesDiscs) {
  const auto japan = identity("resident evil 2", {"JP"}, 1);
  const auto usa = identity("resident evil 2", {"US"}, 1);

  EXPECT_TRUE(areVariants(japan, usa));
  EXPECT_FALSE(areDiscsOfOneRelease(japan, identity("resident evil 2", {"US"}, 2)));
}

// Two copies of one disc have to reach the grouper so it can refuse to guess which release
// each came from, rather than being filtered out of sight here
TEST(GameIdentityTest, RepeatedDiscNumbersStillReadAsOneRelease) {
  EXPECT_TRUE(areDiscsOfOneRelease(identity("parasite eve", {}, 2), identity("parasite eve", {}, 2)));
}

// A field added to the key without being projected would read as zero and quietly stop mattering
TEST(GameIdentityTest, IdentityIsReadFromTheEntryFieldByField) {
  Entry entry;
  entry.platformId = 7;
  entry.normalizedTitle = "resident evil 2";
  entry.metadata.regions = {"US"};

  const auto identity = identityOf(entry);

  EXPECT_EQ(identity.platformId, 7u);
  EXPECT_EQ(identity.title, "resident evil 2");
  EXPECT_EQ(identity.regions, (std::vector<std::string>{"US"}));

  // TODO
  // Which disc a dump is belongs to the file. An entry stands for a game rather than for one of
  // its discs, so there is nothing here to read it from
  EXPECT_EQ(identity.discNumber, 0);
}

} // namespace firelight::library

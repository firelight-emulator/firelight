#include <firelight/library/variant_group.hpp>

#include <gtest/gtest.h>

// Verifies which variant stands for a group. Ranking is strictly lexicographic, so each test moves
// exactly one rung and leaves the rest tied
namespace firelight::library {

namespace {
VariantPreference defaultPreference() { return {{"US", "EU", "JP", "WORLD"}, {"en"}}; }

VariantCandidate candidate(const int entryId, std::vector<std::string> regions) {
  VariantCandidate c;
  c.entryId = entryId;
  c.regions = std::move(regions);
  return c;
}
} // namespace

TEST(VariantPrimaryTest, PrefersTheEarlierRegionInTheOrdering) {
  const std::vector candidates = {candidate(1, {"JP"}), candidate(2, {"US"}), candidate(3, {"EU"})};

  EXPECT_EQ(choosePrimaryEntry(candidates, defaultPreference()), 2);
}

TEST(VariantPrimaryTest, LanguageBreaksARegionTie) {
  auto japanese = candidate(1, {"EU"});
  japanese.languages = {"fr"};
  auto english = candidate(2, {"EU"});
  english.languages = {"en"};

  EXPECT_EQ(choosePrimaryEntry({japanese, english}, defaultPreference()), 2);
}

TEST(VariantPrimaryTest, AReleaseBeatsAPrototypeEvenFromAWorseRegion) {
  auto prototype = candidate(1, {"US"});
  prototype.flags = {"proto"};
  const auto release = candidate(2, {"JP"});

  EXPECT_EQ(choosePrimaryEntry({prototype, release}, defaultPreference()), 2);
}

TEST(VariantPrimaryTest, PrefersTheHigherRevision) {
  auto original = candidate(1, {"US"});
  auto revised = candidate(2, {"US"});
  revised.revision = "1";

  EXPECT_EQ(choosePrimaryEntry({original, revised}, defaultPreference()), 2);

  auto revisedAgain = candidate(3, {"US"});
  revisedAgain.revision = "10";

  // Numeric when both parse, so 10 beats 2 rather than sorting as a string
  auto revisionTwo = candidate(4, {"US"});
  revisionTwo.revision = "2";

  EXPECT_EQ(choosePrimaryEntry({revisionTwo, revisedAgain}, defaultPreference()), 3);
}

TEST(VariantPrimaryTest, FallsBackToLexicographicForNonNumericRevisions) {
  auto revA = candidate(1, {"US"});
  revA.revision = "A";
  auto revB = candidate(2, {"US"});
  revB.revision = "B";

  EXPECT_EQ(choosePrimaryEntry({revA, revB}, defaultPreference()), 2);
}

TEST(VariantPrimaryTest, AnUnknownRegionRanksAfterEveryKnownOne) {
  const std::vector candidates = {candidate(1, {"BR"}), candidate(2, {"WORLD"})};

  EXPECT_EQ(choosePrimaryEntry(candidates, defaultPreference()), 2);
}

TEST(VariantPrimaryTest, TiesBreakOnTheLowestIdSoTheAnswerNeverDependsOnOrder) {
  const std::vector ascending = {candidate(7, {"US"}), candidate(3, {"US"})};
  const std::vector descending = {candidate(3, {"US"}), candidate(7, {"US"})};

  EXPECT_EQ(choosePrimaryEntry(ascending, defaultPreference()), 3);
  EXPECT_EQ(choosePrimaryEntry(descending, defaultPreference()), 3);
}

// A hidden entry cannot be launched, so a group whose best variant is missing falls to the next one
TEST(VariantPrimaryTest, SkipsHiddenCandidates) {
  auto missing = candidate(1, {"US"});
  missing.hidden = true;
  const auto present = candidate(2, {"JP"});

  EXPECT_EQ(choosePrimaryEntry({missing, present}, defaultPreference()), 2);
}

TEST(VariantPrimaryTest, ReturnsNothingWhenThereIsNobodyToChoose) {
  EXPECT_FALSE(choosePrimaryEntry({}, defaultPreference()).has_value());

  auto missing = candidate(1, {"US"});
  missing.hidden = true;

  EXPECT_FALSE(choosePrimaryEntry({missing}, defaultPreference()).has_value());
}

TEST(VariantPrimaryTest, ParsesAPreferenceSetting) {
  EXPECT_EQ(parsePreferenceOrder("US,EU,JP"), (std::vector<std::string>{"US", "EU", "JP"}));
  EXPECT_EQ(parsePreferenceOrder(" US , EU "), (std::vector<std::string>{"US", "EU"}));
  EXPECT_TRUE(parsePreferenceOrder("").empty());
  EXPECT_TRUE(parsePreferenceOrder(" , ").empty());
}

} // namespace firelight::library

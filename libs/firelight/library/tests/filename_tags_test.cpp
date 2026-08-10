#include <firelight/library/filename_tags.hpp>

#include <gtest/gtest.h>

// Verifies what a No-Intro / Redump / GoodTools filename says about a dump. The filename is only ever
// a hint about attributes; identity comes from the content hash
namespace firelight::library {

TEST(FilenameTagsTest, TitleIsEverythingBeforeTheFirstGroup) {
  EXPECT_EQ(parseFilenameTags("Super Mario World (USA).sfc").title, "Super Mario World");
  EXPECT_EQ(parseFilenameTags("Chrono Trigger [!].smc").title, "Chrono Trigger");
}

TEST(FilenameTagsTest, TitleSurvivesAPathAndAnUntaggedName) {
  EXPECT_EQ(parseFilenameTags("C:/roms/snes/Super Metroid (Japan, USA).sfc").title, "Super Metroid");
  EXPECT_EQ(parseFilenameTags("/home/alex/roms/EarthBound.smc").title, "EarthBound");
  EXPECT_EQ(parseFilenameTags("smb3").title, "smb3");
}

// Set dumps number their files. The leading zero is what tells an index apart from a
// title that genuinely starts with a number
TEST(FilenameTagsTest, StripsASetIndexPrefix) {
  EXPECT_EQ(parseFilenameTags("0028 - Kirby - Canvas Curse (U)(Trashman).nds").title, "Kirby - Canvas Curse");
  EXPECT_EQ(parseFilenameTags("0479 - New Super Mario Bros. (E)(Supremacy).nds").title, "New Super Mario Bros.");
  EXPECT_EQ(parseFilenameTags("0001-Some Game.nds").title, "Some Game");
}

TEST(FilenameTagsTest, KeepsANumberThatIsPartOfTheTitle) {
  EXPECT_EQ(parseFilenameTags("1943 - The Battle of Midway (USA).nes").title, "1943 - The Battle of Midway");
  EXPECT_EQ(parseFilenameTags("007 James Bond (Japan).sg").title, "007 James Bond");
  EXPECT_EQ(parseFilenameTags("1942.nes").title, "1942");
}

TEST(FilenameTagsTest, ParsesRegionNames) {
  EXPECT_EQ(parseFilenameTags("Game (USA).sfc").regions, (std::vector<std::string>{"US"}));
  EXPECT_EQ(parseFilenameTags("Game (Europe).sfc").regions, (std::vector<std::string>{"EU"}));
  EXPECT_EQ(parseFilenameTags("Game (Japan).sfc").regions, (std::vector<std::string>{"JP"}));
  EXPECT_EQ(parseFilenameTags("Game (World).sfc").regions, (std::vector<std::string>{"WORLD"}));
  EXPECT_EQ(parseFilenameTags("Game (USA, Europe).sfc").regions, (std::vector<std::string>{"US", "EU"}));
}

TEST(FilenameTagsTest, ParsesGoodToolsSingleLetterRegions) {
  EXPECT_EQ(parseFilenameTags("Game (U).nes").regions, (std::vector<std::string>{"US"}));
  EXPECT_EQ(parseFilenameTags("Game (J).nes").regions, (std::vector<std::string>{"JP"}));
  EXPECT_EQ(parseFilenameTags("Game (E).nes").regions, (std::vector<std::string>{"EU"}));
}

TEST(FilenameTagsTest, ParsesLanguageLists) {
  EXPECT_EQ(parseFilenameTags("Game (En,Fr,De).sfc").languages, (std::vector<std::string>{"en", "fr", "de"}));
  EXPECT_EQ(parseFilenameTags("Game (En, Fr, De).sfc").languages, (std::vector<std::string>{"en", "fr", "de"}));
}

// A two-letter group reads as languages, which is what settles the tokens naming both a region and a
// language. Pinned here so the choice is explicit rather than incidental
TEST(FilenameTagsTest, TwoLetterGroupsAreLanguagesNotRegions) {
  const auto norwegian = parseFilenameTags("Game (No).sfc");
  EXPECT_EQ(norwegian.languages, (std::vector<std::string>{"no"}));
  EXPECT_TRUE(norwegian.regions.empty());

  const auto italian = parseFilenameTags("Game (It).sfc");
  EXPECT_EQ(italian.languages, (std::vector<std::string>{"it"}));
  EXPECT_TRUE(italian.regions.empty());

  // Two letters that name no language stay a region
  const auto europe = parseFilenameTags("Game (EU).sfc");
  EXPECT_EQ(europe.regions, (std::vector<std::string>{"EU"}));
  EXPECT_TRUE(europe.languages.empty());
}

TEST(FilenameTagsTest, ParsesRevisionAndVersion) {
  EXPECT_EQ(parseFilenameTags("Game (Rev 1).sfc").revision, "1");
  EXPECT_EQ(parseFilenameTags("Game (Rev A).sfc").revision, "A");
  EXPECT_EQ(parseFilenameTags("Game (v1.1).sfc").version, "1.1");
}

TEST(FilenameTagsTest, ParsesReleaseStateFlags) {
  EXPECT_EQ(parseFilenameTags("Game (Beta).sfc").flags, (std::vector<std::string>{"beta"}));
  EXPECT_EQ(parseFilenameTags("Game (Beta 2).sfc").flags, (std::vector<std::string>{"beta"}));
  EXPECT_EQ(parseFilenameTags("Game (Proto).sfc").flags, (std::vector<std::string>{"proto"}));
  EXPECT_EQ(parseFilenameTags("Game (Prototype).sfc").flags, (std::vector<std::string>{"proto"}));
  EXPECT_EQ(parseFilenameTags("Game (Demo).sfc").flags, (std::vector<std::string>{"demo"}));
  EXPECT_EQ(parseFilenameTags("Game (Unl).sfc").flags, (std::vector<std::string>{"unlicensed"}));
}

TEST(FilenameTagsTest, ParsesDumpFlags) {
  EXPECT_EQ(parseFilenameTags("Game [!].smc").flags, (std::vector<std::string>{"verified-dump"}));
  EXPECT_EQ(parseFilenameTags("Game [b].smc").flags, (std::vector<std::string>{"bad-dump"}));
  EXPECT_EQ(parseFilenameTags("Game [b1].smc").flags, (std::vector<std::string>{"bad-dump"}));
  EXPECT_EQ(parseFilenameTags("Game [a].smc").flags, (std::vector<std::string>{"alt"}));
  EXPECT_EQ(parseFilenameTags("Game [o].smc").flags, (std::vector<std::string>{"overdump"}));
}

TEST(FilenameTagsTest, ParsesTranslations) {
  const auto upToDate = parseFilenameTags("Game [T+En].smc");
  EXPECT_TRUE(upToDate.isTranslation);
  EXPECT_EQ(upToDate.translationLanguage, "en");
  EXPECT_EQ(upToDate.flags, (std::vector<std::string>{"translation"}));

  // T- is an outdated translation, but a translation all the same
  const auto outdated = parseFilenameTags("Game [T-Fr].smc");
  EXPECT_TRUE(outdated.isTranslation);
  EXPECT_EQ(outdated.translationLanguage, "fr");
}

// Disc numbers are recorded but never read as a region, so the multi-disc work has them waiting
TEST(FilenameTagsTest, DiscNumberIsCapturedAndIsNotARegion) {
  const auto tags = parseFilenameTags("Final Fantasy VII (USA) (Disc 1).cue");
  EXPECT_EQ(tags.discNumber, 1);
  EXPECT_EQ(tags.regions, (std::vector<std::string>{"US"}));

  EXPECT_EQ(parseFilenameTags("Game (Disk 2).cue").discNumber, 2);
  EXPECT_EQ(parseFilenameTags("Game (CD 3).cue").discNumber, 3);
}

TEST(FilenameTagsTest, ParsesAFullyLoadedName) {
  const auto tags = parseFilenameTags("Some Game (USA, Europe) (En,Fr,De) (Rev 1) [!].zip");

  EXPECT_EQ(tags.title, "Some Game");
  EXPECT_EQ(tags.regions, (std::vector<std::string>{"US", "EU"}));
  EXPECT_EQ(tags.languages, (std::vector<std::string>{"en", "fr", "de"}));
  EXPECT_EQ(tags.revision, "1");
  EXPECT_EQ(tags.flags, (std::vector<std::string>{"verified-dump"}));
}

// Whatever follows the tag groups is part of what the file is called. A patched dump and
// the dump it came from are different games as far as the library is concerned
TEST(FilenameTagsTest, KeepsTextAfterTheTagGroups) {
  EXPECT_EQ(parseFilenameTags("Super Mario World (USA)-patched.sfc").title, "Super Mario World-patched");
  EXPECT_EQ(parseFilenameTags("Chrono Trigger (USA) [!] - patched.smc").title, "Chrono Trigger - patched");
  EXPECT_EQ(parseFilenameTags("Game (USA) v2.sfc").title, "Game v2");

  // Which means they do not read as the same game
  EXPECT_NE(normalizeTitle(parseFilenameTags("Super Mario World (USA).sfc").title),
            normalizeTitle(parseFilenameTags("Super Mario World (USA)-patched.sfc").title));
}

// Text between two groups is kept too, not just text after the last one
TEST(FilenameTagsTest, KeepsTextBetweenTagGroups) {
  EXPECT_EQ(parseFilenameTags("Game (USA) Deluxe (Rev 1).sfc").title, "Game Deluxe");
  EXPECT_EQ(parseFilenameTags("Game (USA)  (Rev 1).sfc").title, "Game");
}

TEST(FilenameTagsTest, ToleratesMalformedNames) {
  EXPECT_NO_THROW(parseFilenameTags("Game (USA.sfc"));
  EXPECT_NO_THROW(parseFilenameTags("Game [.smc"));
  EXPECT_NO_THROW(parseFilenameTags(""));
  EXPECT_NO_THROW(parseFilenameTags("()"));

  EXPECT_EQ(parseFilenameTags("Pokémon Rouge (France).gb").title, "Pokémon Rouge");
}

// Two dumps of one game must fold to the same string, and two different games must not
TEST(NormalizeTitleTest, FoldsCaseAndSpacingButNotPunctuation) {
  EXPECT_EQ(normalizeTitle("Zelda"), "zelda");
  EXPECT_EQ(normalizeTitle("  The   Legend of Zelda  "), "the legend of zelda");
  EXPECT_EQ(normalizeTitle("ZELDA"), normalizeTitle("zelda"));
  EXPECT_EQ(normalizeTitle(""), "");

  // Different games, and they stay different
  EXPECT_NE(normalizeTitle("Zelda"), normalizeTitle("Zelda II"));
  EXPECT_NE(normalizeTitle("Final Fantasy IV"), normalizeTitle("Final Fantasy II"));

  // Punctuation is left alone, so this is a same-title test rather than a similar-title one
  EXPECT_NE(normalizeTitle("Zelda: A Link to the Past"), normalizeTitle("Zelda - A Link to the Past"));
}

} // namespace firelight::library

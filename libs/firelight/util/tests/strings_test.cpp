#include <firelight/util/strings.hpp>

#include <gtest/gtest.h>

// The pieces title keys are built from. Each one does exactly one thing, so a policy can
// take the folds it wants without inheriting the ones it does not
namespace firelight::strings {

TEST(StringsTest, ToLower) {
  EXPECT_EQ(toLower("ZELDA"), "zelda");
  EXPECT_EQ(toLower("MiXeD 123"), "mixed 123");
  EXPECT_EQ(toLower(""), "");
}

TEST(StringsTest, Trim) {
  EXPECT_EQ(trim("  zelda  "), "zelda");
  EXPECT_EQ(trim("\t\n zelda \r\n"), "zelda");
  EXPECT_EQ(trim("zelda"), "zelda");
  EXPECT_EQ(trim("   "), "");
  EXPECT_EQ(trim(""), "");
}

TEST(StringsTest, CollapseWhitespace) {
  EXPECT_EQ(collapseWhitespace("  the   legend  of zelda "), "the legend of zelda");
  EXPECT_EQ(collapseWhitespace("zelda"), "zelda");
  EXPECT_EQ(collapseWhitespace("   "), "");

  // Every kind of whitespace becomes the same single space
  EXPECT_EQ(collapseWhitespace("a\tb\nc"), "a b c");
}

TEST(StringsTest, ReplaceAll) {
  EXPECT_EQ(replaceAll("a-b-c", "-", "+"), "a+b+c");
  EXPECT_EQ(replaceAll("aaa", "aa", "b"), "ba");
  EXPECT_EQ(replaceAll("zelda", "x", "y"), "zelda");

  // An empty needle would otherwise match forever
  EXPECT_EQ(replaceAll("zelda", "", "x"), "zelda");
}

TEST(StringsTest, Split) {
  EXPECT_EQ(split("a,b,c", ','), (std::vector<std::string>{"a", "b", "c"}));
  EXPECT_EQ(split(" a , b ", ','), (std::vector<std::string>{"a", "b"}));
  EXPECT_EQ(split("a,,b", ','), (std::vector<std::string>{"a", "b"}));
  EXPECT_EQ(split("a,,b", ',', true), (std::vector<std::string>{"a", "", "b"}));
  EXPECT_TRUE(split("", ',').empty());
}

TEST(StringsTest, Join) {
  EXPECT_EQ(join({"a", "b", "c"}, ", "), "a, b, c");
  EXPECT_EQ(join({"a"}, ", "), "a");
  EXPECT_EQ(join({}, ", "), "");
}

TEST(StringsTest, SplitAndJoinRoundTrip) { EXPECT_EQ(join(split("US, EU, JP", ','), ","), "US,EU,JP"); }

TEST(StringsTest, StartsWith) {
  EXPECT_TRUE(startsWith("zelda", "zel"));
  EXPECT_TRUE(startsWith("zelda", "zelda"));
  EXPECT_TRUE(startsWith("zelda", ""));
  EXPECT_FALSE(startsWith("zelda", "Zel"));
  EXPECT_FALSE(startsWith("zel", "zelda"));
}

TEST(StringsTest, StartsWithIgnoringCase) {
  EXPECT_TRUE(startsWithIgnoringCase("Rev 1", "rev "));
  EXPECT_TRUE(startsWithIgnoringCase("BETA 2", "beta"));
  EXPECT_FALSE(startsWithIgnoringCase("Revision", "rev "));
  EXPECT_FALSE(startsWithIgnoringCase("Re", "rev "));
}

TEST(StringsTest, EndsWithIgnoringCase) {
  EXPECT_TRUE(endsWithIgnoringCase("game.chd", ".chd"));
  EXPECT_TRUE(endsWithIgnoringCase("GAME.CHD", ".chd"));
  EXPECT_TRUE(endsWithIgnoringCase("game.Chd", ".CHD"));
  EXPECT_FALSE(endsWithIgnoringCase("game.cue", ".chd"));
  EXPECT_FALSE(endsWithIgnoringCase("chd", ".chd"));
  EXPECT_TRUE(endsWithIgnoringCase("anything", ""));
}

TEST(StringsTest, Contains) {
  EXPECT_TRUE(contains("the legend of zelda", "legend"));
  EXPECT_FALSE(contains("zelda", "Zelda"));
  EXPECT_TRUE(contains("zelda", ""));
}

TEST(StringsTest, ContainsIgnoringCase) {
  EXPECT_TRUE(containsIgnoringCase("The Legend of Zelda", "LEGEND"));
  EXPECT_TRUE(containsIgnoringCase("zelda", "ZEL"));
  EXPECT_FALSE(containsIgnoringCase("zelda", "mario"));

  // An empty needle matches anything, so a blank search box filters nothing out
  EXPECT_TRUE(containsIgnoringCase("zelda", ""));
  EXPECT_TRUE(containsIgnoringCase("", ""));
}

// "Ratchet & Clank" and "Ratchet and Clank" are the same game
TEST(StringsTest, FoldAmpersand) {
  EXPECT_EQ(foldAmpersand("Ratchet & Clank"), "Ratchet and Clank");
  EXPECT_EQ(foldAmpersand("Tom & Jerry"), "Tom and Jerry");
  EXPECT_EQ(foldAmpersand("&Start"), "andStart");

  // Glued between two characters it is part of a name, not the word
  EXPECT_EQ(foldAmpersand("AT&T"), "AT&T");
  EXPECT_EQ(foldAmpersand("R&B"), "R&B");
}

TEST(StringsTest, StripPunctuation) {
  EXPECT_EQ(stripPunctuation("Zelda: A Link to the Past"), "Zelda A Link to the Past");
  EXPECT_EQ(stripPunctuation("Mega Man X4"), "Mega Man X4");
  EXPECT_EQ(stripPunctuation("!@#$"), "");

  // Spaces survive, so words do not run together
  EXPECT_EQ(stripPunctuation("a - b"), "a  b");
}

// Set dumps write the article at the end
TEST(StringsTest, RestoreTrailingArticle) {
  EXPECT_EQ(restoreTrailingArticle("Legend of Zelda, The"), "The Legend of Zelda");
  EXPECT_EQ(restoreTrailingArticle("Bug's Life, A"), "A Bug's Life");
  EXPECT_EQ(restoreTrailingArticle("Zelda"), "Zelda");

  // Only an article moves; a subtitle after a comma stays where it is
  EXPECT_EQ(restoreTrailingArticle("Sonic, Knuckles"), "Sonic, Knuckles");
}

TEST(StringsTest, StripLeadingArticle) {
  EXPECT_EQ(stripLeadingArticle("The Legend of Zelda"), "Legend of Zelda");
  EXPECT_EQ(stripLeadingArticle("A Bug's Life"), "Bug's Life");
  EXPECT_EQ(stripLeadingArticle("An American Tail"), "American Tail");
  EXPECT_EQ(stripLeadingArticle("Zelda"), "Zelda");

  // A title that only starts with those letters keeps them
  EXPECT_EQ(stripLeadingArticle("Theme Park"), "Theme Park");
  EXPECT_EQ(stripLeadingArticle("Antarctic Adventure"), "Antarctic Adventure");
}

TEST(StringsTest, FoldRomanNumerals) {
  EXPECT_EQ(foldRomanNumerals("Final Fantasy IV"), "Final Fantasy 4");
  EXPECT_EQ(foldRomanNumerals("Final Fantasy vii"), "Final Fantasy 7");
  EXPECT_EQ(foldRomanNumerals("Rocky XX"), "Rocky 20");

  // Only a whole word counts
  EXPECT_EQ(foldRomanNumerals("Civilization"), "Civilization");
  EXPECT_EQ(foldRomanNumerals("Virtua Racing"), "Virtua Racing");

  // Two sequels stay apart, which is why this belongs in a matching key and not a
  // grouping key
  EXPECT_NE(foldRomanNumerals("Final Fantasy IV"), foldRomanNumerals("Final Fantasy V"));
}

} // namespace firelight::strings

#include <firelight/library/disc_set.hpp>

#include <gtest/gtest.h>

// What a set's shape can be read from the discs present. There is no external database saying
// how many discs a game shipped on, so these only report what the numbers themselves show
namespace firelight::library {

TEST(DiscSetTest, TheLowestDiscStandsForTheSet) {
  EXPECT_EQ(representativeDisc({3, 1, 2}), 1);
  EXPECT_EQ(representativeDisc({2, 3}), 2);
  EXPECT_EQ(representativeDisc({1}), 1);
  EXPECT_EQ(representativeDisc({}), 0);

  // A disc number of 0 means "not part of a set" and is not a candidate
  EXPECT_EQ(representativeDisc({0, 0}), 0);
}

TEST(DiscSetTest, ReadsCompleteness) {
  EXPECT_EQ(discSetState({1, 2, 3}), DiscSetState::Complete);
  EXPECT_EQ(discSetState({1}), DiscSetState::Complete);
  EXPECT_EQ(discSetState({3, 2, 1}), DiscSetState::Complete);

  EXPECT_EQ(discSetState({2, 3}), DiscSetState::MissingFirstDisc);
  EXPECT_EQ(discSetState({}), DiscSetState::MissingFirstDisc);

  EXPECT_EQ(discSetState({1, 3}), DiscSetState::MissingMiddleDisc);
  EXPECT_EQ(discSetState({1, 2, 4}), DiscSetState::MissingMiddleDisc);
}

// The same disc found in two folders is still one disc
TEST(DiscSetTest, DuplicatesDoNotCountTwice) {
  EXPECT_EQ(discSetState({1, 2, 2, 3}), DiscSetState::Complete);
  EXPECT_EQ(representativeDisc({2, 2}), 2);
}

TEST(DiscSetTest, NamesTheMissingDiscs) {
  EXPECT_TRUE(missingDiscs({1, 2, 3}).empty());
  EXPECT_EQ(missingDiscs({1, 3}), (std::vector<int>{2}));
  EXPECT_EQ(missingDiscs({2, 4}), (std::vector<int>{1, 3}));

  // Nothing is claimed beyond the highest disc present, because nothing is known about it
  EXPECT_EQ(missingDiscs({2}), (std::vector<int>{1}));
  EXPECT_TRUE(missingDiscs({}).empty());
}

// Disc 1 is what makes a set launchable. Without it the identity could move when disc 1
// arrived, and playing disc 2 of an RPG on its own is not a use case
TEST(DiscSetTest, LaunchableOnlyWithDiscOne) {
  EXPECT_TRUE(isDiscSetLaunchable({1}));
  EXPECT_TRUE(isDiscSetLaunchable({1, 2, 3}));
  EXPECT_TRUE(isDiscSetLaunchable({1, 3}));

  EXPECT_FALSE(isDiscSetLaunchable({2, 3}));
  EXPECT_FALSE(isDiscSetLaunchable({}));
}

} // namespace firelight::library

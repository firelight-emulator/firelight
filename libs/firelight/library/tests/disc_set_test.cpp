// TODO: NEEDS REVIEW
#include <firelight/library/disc_set.hpp>

#include <gtest/gtest.h>

// TODO
// Which discs a set is short of. The numbers on hand answer that up to the highest one present;
// past that only a count from somewhere authoritative can
namespace firelight::library {

TEST(DiscSetTest, NamesTheMissingDiscs) {
  EXPECT_TRUE(missingDiscs({1, 2, 3}).empty());
  EXPECT_EQ(missingDiscs({1, 3}), (std::vector<int>{2}));
  EXPECT_EQ(missingDiscs({2, 4}), (std::vector<int>{1, 3}));

  // Nothing is claimed beyond the highest disc present, because nothing is known about it
  EXPECT_EQ(missingDiscs({2}), (std::vector<int>{1}));
  EXPECT_TRUE(missingDiscs({}).empty());
}

// TODO
// The same disc found in two folders is still one disc, and a file carrying no number is not a
// disc of anything
TEST(DiscSetTest, DuplicatesAndUnnumberedFilesDoNotDisturbTheRun) {
  EXPECT_TRUE(missingDiscs({1, 2, 2, 3}).empty());
  EXPECT_EQ(missingDiscs({1, 3, 3}), (std::vector<int>{2}));
  EXPECT_TRUE(missingDiscs({0, 1}).empty());
}

// A set short of its last disc has no hole to find, so the run alone reads as complete. Only a
// count from somewhere authoritative can say otherwise
TEST(DiscSetTest, AKnownCountNamesTheDiscsPastTheOnesOnHand) {
  EXPECT_TRUE(missingDiscs({1, 2}).empty());
  EXPECT_EQ(missingDiscs({1, 2}, 3), (std::vector<int>{3}));
  EXPECT_EQ(missingDiscs({1}, 3), (std::vector<int>{2, 3}));

  // Holes and the tail are the same question once the count is known
  EXPECT_EQ(missingDiscs({2}, 3), (std::vector<int>{1, 3}));

  // A count that claims less than is on hand does not take discs away
  EXPECT_TRUE(missingDiscs({1, 2, 3}, 2).empty());
  EXPECT_EQ(missingDiscs({1, 3}, 2), (std::vector<int>{2}));

  // Owning nothing says nothing, whatever the count claims
  EXPECT_TRUE(missingDiscs({}, 3).empty());
}

} // namespace firelight::library

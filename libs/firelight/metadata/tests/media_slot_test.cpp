#include <firelight/metadata/media_slot.hpp>

#include <gtest/gtest.h>

// Verifies which image a slot shows. The ladder decides which type to fall back to; selection decides
// between assets of a type
namespace firelight::metadata {

namespace {
MediaAsset asset(const int id, const MediaType type, const bool selected = false) {
  MediaAsset a;
  a.id = id;
  a.contentHash = "hash";
  a.type = type;
  a.selected = selected;
  a.remoteUrl = "http://example.invalid/" + std::to_string(id);
  return a;
}
} // namespace

TEST(MediaSlotTest, PrefersTheFirstTypeInTheLadder) {
  const std::vector assets = {asset(1, MediaType::Ingame), asset(2, MediaType::Icon), asset(3, MediaType::GridSquare)};

  const auto resolved = resolveSlot(assets, MediaSlot::TileSquare);

  ASSERT_TRUE(resolved.has_value());
  EXPECT_EQ(resolved->id, 3);
}

// The point of the ladder: art the game does have beats a blank tile
TEST(MediaSlotTest, FallsDownTheLadderWhenTheBestTypeIsMissing) {
  const std::vector assets = {asset(1, MediaType::TitleScreen)};

  const auto resolved = resolveSlot(assets, MediaSlot::TileSquare);

  ASSERT_TRUE(resolved.has_value());
  EXPECT_EQ(resolved->id, 1);
}

TEST(MediaSlotTest, SelectionDecidesBetweenAssetsOfTheSameType) {
  const std::vector assets = {asset(1, MediaType::GridSquare), asset(2, MediaType::GridSquare, true),
                              asset(3, MediaType::GridSquare)};

  const auto resolved = resolveSlot(assets, MediaSlot::TileSquare);

  ASSERT_TRUE(resolved.has_value());
  EXPECT_EQ(resolved->id, 2);
}

// Selection does not promote a type past the ladder — a chosen screenshot is still a screenshot
TEST(MediaSlotTest, SelectionDoesNotOutrankTheLadder) {
  const std::vector assets = {asset(1, MediaType::Ingame, true), asset(2, MediaType::GridSquare)};

  const auto resolved = resolveSlot(assets, MediaSlot::TileSquare);

  ASSERT_TRUE(resolved.has_value());
  EXPECT_EQ(resolved->id, 2);
}

TEST(MediaSlotTest, EachSlotHasItsOwnLadder) {
  const std::vector assets = {asset(1, MediaType::GridSquare), asset(2, MediaType::GridBanner)};

  const auto tile = resolveSlot(assets, MediaSlot::TileSquare);
  ASSERT_TRUE(tile.has_value());
  EXPECT_EQ(tile->id, 1);

  const auto banner = resolveSlot(assets, MediaSlot::TileBanner);
  ASSERT_TRUE(banner.has_value());
  EXPECT_EQ(banner->id, 2);

  const auto hero = resolveSlot(assets, MediaSlot::Hero);
  ASSERT_TRUE(hero.has_value());
  EXPECT_EQ(hero->id, 2);

  // Nothing in the back-boxart ladder, so nothing to show
  EXPECT_FALSE(resolveSlot(assets, MediaSlot::BoxartBack).has_value());
}

TEST(MediaSlotTest, ReturnsNothingWhenTheGameHasNoArtAtAll) {
  EXPECT_FALSE(resolveSlot({}, MediaSlot::TileSquare).has_value());
}

} // namespace firelight::metadata

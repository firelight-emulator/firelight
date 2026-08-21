#pragma once

#include <firelight/metadata/media_asset.hpp>

#include <optional>
#include <vector>

namespace firelight::metadata {

/**
 * A place the UI needs an image, as opposed to what an image is.
 *
 * A slot accepts several media types in order of preference, so a game with a title screen and no
 * icon still shows something in the grid
 */
enum class MediaSlot {
  // One per grid shape, because a tile laid out for one aspect cannot use art cut
  // for another
  TileSquare,   // 1:1
  TilePortrait, // 2:3
  TileVertical, // 22:31
  TileBanner,   // 92:43
  BoxartFront,
  BoxartBack,
  Hero,
  ClearLogo,
  TitleScreen
};

/**
 * The media types a slot accepts, best first
 */
[[nodiscard]] const std::vector<MediaType> &ladderFor(MediaSlot slot);

/**
 * The asset to show in a slot, or nothing when none of its types has one.
 *
 * Takes the first type in the slot's ladder that has any asset at all, and within that type prefers
 * the one the user selected. Selection therefore decides between assets of a type, and the ladder
 * only decides which type to fall back to
 */
[[nodiscard]] std::optional<MediaAsset> resolveSlot(const std::vector<MediaAsset> &assets, MediaSlot slot);

} // namespace firelight::metadata

#pragma once

#include <firelight/util/game_metadata.hpp>

#include <string>
#include <vector>

namespace firelight::metadata {
/**
 * Represents a type of media asset. Don't change the numbers since they are persisted in the database. :)
 */
enum class MediaType {
  Icon = 0, // Small square app-style icon, not the grid tile
  BoxartFront = 1,
  BoxartBack = 2,
  Logo = 3, // Clear/transparent logo
  Hero = 4, // Wide background / banner art
  // The four shapes a grid comes in. Each is its own type because a tile laid out
  // for one aspect cannot use art cut for another
  GridPortrait = 5, // 2:3
  GridBanner = 6,   // 92:43
  TitleScreen = 7,
  Ingame = 8,
  GridSquare = 9,    // 1:1
  GridVertical = 10, // 22:31
};

/**
 * A default media asset for a game from the metadata source
 */
struct MediaDefault {
  MediaType type = MediaType::Icon;
  std::string url;
};

/**
 * One source's answer about a game.
 *
 * The facts sit in metadata, in the same shape a library entry stores, so nothing translates between
 * the two. What is beside them belongs to the lookup rather than to the game: a name the entry may or
 * may not adopt, artwork that lives in its own store, and the ids this source knows it by
 */
struct MetadataLookup {
  std::string name;
  GameMetadata metadata;
  std::vector<MediaDefault> media;
  unsigned retroAchievementsId = 0;
  int platformId = -1;
};

} // namespace firelight::metadata

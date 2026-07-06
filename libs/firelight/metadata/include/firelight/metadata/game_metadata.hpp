#pragma once

#include <string>
#include <vector>

namespace firelight::metadata {
  /**
   * Represents a type of media asset. Don't change the numbers since they are persisted in the database. :)
   */
  enum class MediaType {
    Icon = 0, // Square-ish game icon (library grid tile)
    BoxartFront = 1,
    BoxartBack = 2,
    Logo = 3, // Clear/transparent logo
    Hero = 4, // Wide background / banner art
    GridPortrait = 5, // 2x3 portrait grid
    GridBanner = 6, // ~92x43 banner grid
    TitleScreen = 7,
    Ingame = 8,
  };

  /**
   * A default media asset for a game from the metadata source.
   */
  struct MediaDefault {
    MediaType type = MediaType::Icon;
    std::string url;
  };

  /**
   * Set of metadata for a game. This isn't what gets stored in each library entry, it's from our metadata source and
   * used to populate the library entry when the user adds a game.
   */
  struct GameMetadata {
    std::string name;
    std::string description;
    std::string developer;
    std::string publisher;
    std::string genre; // free-form (may be several, comma/semicolon separated)
    unsigned releaseYear = 0;
    std::string releaseDate; // ISO-ish date if available
    std::string region;
    std::string players;
    unsigned retroAchievementsId = 0;
    int platformId = -1;
    std::vector<MediaDefault> media;
  };
}

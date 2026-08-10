#pragma once

#include <firelight/metadata/game_metadata.hpp>

#include <cstdint>
#include <string>

namespace firelight::metadata {
/**
 * Represents the place a media asset can come from
 */
enum class MediaSource {
  RetroAchievements,
  SteamGridDb,
  User,
  Unknown,
};

[[nodiscard]] const char *toString(MediaSource source);

[[nodiscard]] MediaSource mediaSourceFromString(const std::string &value);

/**
 * A single media asset for a game that is actually stored somewhere (local or remote) and can be displayed in
 * the UI. This is what gets created/stored when a user selects an asset from a search result, and is what gets
 * displayed in the UI
 */
struct MediaAsset {
  int id = -1;
  std::string contentHash;
  MediaType type = MediaType::Icon;
  MediaSource source = MediaSource::Unknown;
  std::string remoteUrl;
  std::string thumbUrl;
  std::string localPath;
  int width = 0;
  int height = 0;
  std::string externalId; // Provider-specific id (SteamGridDB asset id, etc.)
  bool selected = false;
  // True when this came from matching a title rather than the content hash, so a
  // review screen can find the guesses among the certain ones
  bool unconfirmed = false;
  // What the provider thought the game was, and how well that answered the query.
  // Kept because "La Wares -> La-Mulana" is only obviously wrong once you can see it
  std::string matchedName;
  int matchScore = 0;
  uint64_t createdAt = 0;

  /**
   * @return The source used to actually display the asset
   */
  [[nodiscard]] std::string displaySource() const { return !remoteUrl.empty() ? remoteUrl : localPath; }

  /**
   * @return The source used to display a small version of the asset (thumbnail)
   *   If no thumbnail is available, falls back to the display source
   */
  [[nodiscard]] std::string displayThumb() const { return !thumbUrl.empty() ? thumbUrl : displaySource(); }
};
} // namespace firelight::metadata

#pragma once

#include <firelight/metadata/game_metadata.hpp>

#include <string>
#include <vector>

namespace firelight::metadata {
/**
 * Represents a single artwork candidate returned by an art provider. When the user selects one, it gets stored
 * as a MediaAsset in the database
 */
struct ArtCandidate {
  MediaType type = MediaType::Icon;
  std::string url;      // Full-resolution image URL
  std::string thumbUrl; // Smaller preview for the picker grid (may equal url)
  int width = 0;
  int height = 0;
  std::string externalId; // Provider-specific asset id
  std::string gameName;   // Name of the game this artwork is for (may differ from the search query)
  // How well gameName answered the query: 3 exact, 2 prefix, 1 substring, 0 unrelated.
  // A low score is the signal that a match is worth a human looking at
  int matchScore = 0;
};

/**
 * Why a search came back the way it did.
 *
 * A search that found nothing and a search that could not be made look the same in the results, and
 * they must not be treated the same: one is an answer, the other is worth asking again later
 */
enum class ArtSearchStatus {
  Ok,
  RateLimited,
  Failed,
};

/**
 * What a search produced, and whether it actually ran
 */
struct ArtSearchResult {
  ArtSearchStatus status = ArtSearchStatus::Ok;
  std::vector<ArtCandidate> candidates;

  [[nodiscard]] bool ok() const { return status == ArtSearchStatus::Ok; }
};

/**
 * Provides artwork candidates for a game. Implementations may fetch from a remote API or local database
 */
class IArtProvider {
public:
  virtual ~IArtProvider() = default;

  /**
   * @return The name of the provider (e.g. "TheGamesDB", "IGDB", "SteamGridDB"). Used for logging and in the UI
   */
  [[nodiscard]] virtual std::string name() const = 0;

  /**
   * @return True if the provider is configured and ready to be used (has an API key, etc.), false otherwise
   */
  [[nodiscard]] virtual bool isConfigured() const = 0;

  /**
   * @param gameName The name of the game to search for artwork candidates
   * @param platformId The platform ID, not always applied but good to include for more accurate results. Use 0 if
   * unknown
   * @param type The type of artwork to search for (icon, screenshot, etc.)
   * @return The artwork candidates, and whether the search ran at all. An empty list with an Ok
   *   status means the provider has nothing; any other status means the question went unanswered
   */
  [[nodiscard]] virtual ArtSearchResult search(const std::string &gameName, int platformId, MediaType type) = 0;
};
} // namespace firelight::metadata

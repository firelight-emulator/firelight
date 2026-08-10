#pragma once

#include <string>
#include <vector>

namespace firelight {

/**
 * The canonical metadata field names.
 *
 * The same strings key the stored document, name the entries in an override set, and identify what a
 * caller changed, so they live in one place rather than as literals at each site
 */
namespace metadata_fields {
inline constexpr auto DESCRIPTION = "description";
inline constexpr auto DEVELOPER = "developer";
inline constexpr auto PUBLISHER = "publisher";
inline constexpr auto RELEASE_YEAR = "releaseYear";
inline constexpr auto RELEASE_DATE = "releaseDate";
inline constexpr auto PLAYERS = "players";
inline constexpr auto REVISION = "revision";
inline constexpr auto GENRES = "genres";
inline constexpr auto REGIONS = "regions";
inline constexpr auto LANGUAGES = "languages";
inline constexpr auto FLAGS = "flags";
} // namespace metadata_fields

/**
 * What is known about one dump of a game.
 *
 * The same shape whether it came from a metadata source, from the filename, or from the user, so a
 * library entry stores it and a source returns it without either translating for the other.
 * Multi-valued fields are ordered most authoritative first.
 *
 * The name is not here: that is user state, and lives on the entry. Nor is anything about the
 * lookup itself, which travels beside this rather than inside it
 */
struct GameMetadata {
  std::string description;
  std::string developer;
  std::string publisher;
  unsigned releaseYear = 0;
  std::string releaseDate;
  std::string players;
  std::string revision;
  std::vector<std::string> genres;
  std::vector<std::string> regions;
  std::vector<std::string> languages;
  std::vector<std::string> flags;

  /**
   * Reads a document. Malformed input yields an empty value rather than throwing
   */
  [[nodiscard]] static GameMetadata parse(const std::string &json);

  /**
   * @return The document, with empty fields omitted
   */
  [[nodiscard]] std::string toJson() const;

  [[nodiscard]] bool isEmpty() const;
};

} // namespace firelight

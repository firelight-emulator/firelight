#pragma once

#include <string>
#include <vector>

namespace firelight {

/**
 * The canonical metadata field names
 */
namespace metadata_fields {
inline constexpr auto DESCRIPTION = "description";
inline constexpr auto DEVELOPER = "developer";
inline constexpr auto PUBLISHER = "publisher";
inline constexpr auto RELEASE_YEAR = "releaseYear";
inline constexpr auto RELEASE_DATE = "releaseDate";
inline constexpr auto PLAYERS = "players";
inline constexpr auto DISC_COUNT = "discCount";
inline constexpr auto REVISION = "revision";
inline constexpr auto GENRES = "genres";
inline constexpr auto REGIONS = "regions";
inline constexpr auto LANGUAGES = "languages";
inline constexpr auto FLAGS = "flags";
} // namespace metadata_fields

/**
 * What is known about one dump of a game. Doesn't have name because that lives on the library entry
 */
struct GameMetadata {
  std::string description;
  std::string developer;
  std::string publisher;
  unsigned releaseYear = 0;
  std::string releaseDate;
  std::string players;
  // How many discs the game came on, 0 when nothing has said
  int discCount = 0;
  std::string revision;
  std::vector<std::string> genres;
  std::vector<std::string> regions;
  std::vector<std::string> languages;
  std::vector<std::string> flags;

  /**
   * Reads serialized GameMetadata, malformed input returns an empty value rather than throwing
   */
  [[nodiscard]] static GameMetadata parse(const std::string &json);

  /**
   * @return The serialized GameMetadata, with empty fields omitted
   */
  [[nodiscard]] std::string toJson() const;

  [[nodiscard]] bool isEmpty() const;
};

} // namespace firelight

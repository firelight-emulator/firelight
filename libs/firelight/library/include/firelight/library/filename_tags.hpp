#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace firelight::library {

/**
 * The flag names parseFilenameTags produces
 */
namespace content_flags {
inline constexpr auto BETA = "beta";
inline constexpr auto PROTO = "proto";
inline constexpr auto DEMO = "demo";
inline constexpr auto SAMPLE = "sample";
inline constexpr auto UNLICENSED = "unlicensed";
inline constexpr auto ALT = "alt";
inline constexpr auto VERIFIED_DUMP = "verified-dump";
inline constexpr auto BAD_DUMP = "bad-dump";
inline constexpr auto OVERDUMP = "overdump";
inline constexpr auto TRAINED = "trained";
inline constexpr auto TRANSLATION = "translation";
inline constexpr auto HACK = "hack";
inline constexpr auto PIRATE = "pirate";
} // namespace content_flags

/**
 * What a No-Intro, Redump, GoodTools or TOSEC style filename says about its content.
 *
 * The filename is a hint about a dump's attributes, never about its identity — the content hash
 * decides what a file is
 */
struct FilenameTags {
  std::string title;
  std::vector<std::string> regions;
  std::vector<std::string> languages;
  std::string revision;
  std::string version;
  std::vector<std::string> flags;
  std::string translationLanguage;
  bool isTranslation = false;
  int discNumber = 0;
};

/**
 * Parses the tag groups out of a filename or a full path
 */
[[nodiscard]] FilenameTags parseFilenameTags(std::string_view fileName);

/**
 * Folds a title to the form two releases of the same game share: lowercased, with runs of
 * whitespace collapsed and the ends trimmed. Punctuation is left alone, so this matches
 * titles that are the same rather than titles that are merely similar
 */
[[nodiscard]] std::string normalizeTitle(const std::string &title);

/**
 * Which of the paths holding one dump an entry reads its name and tags from.
 *
 * Files sharing a content hash are copies of one dump, so the only thing to choose between is
 * how much their names say. Ties break on the path so the same files give the same answer
 * whatever order they were catalogued in
 */
[[nodiscard]] std::string chooseIdentityPath(const std::vector<std::string> &paths);

} // namespace firelight::library

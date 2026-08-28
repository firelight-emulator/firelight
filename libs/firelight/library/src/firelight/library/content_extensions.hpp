// TODO: NEEDS REVIEW
#pragma once

#include <algorithm>
#include <array>
#include <cctype>
#include <string>
#include <string_view>

namespace firelight::library {

// Disc-image extensions are ambiguous: the same extension is used by many
// consoles, so the platform can only be determined by inspecting the disc's
// contents (rcheevos AUTO detection). These classifiers are platform-independent
// file-type facts. Cartridge (rom) extensions map to a platform and are resolved
// via PlatformService::platformIdForExtension instead

// TODO
// Arrays rather than a chain of comparisons because the set has to be enumerable: a predicate
// cannot be asked what it accepts, which is why the test re-typed the list as a second copy
// that drifted from this one
inline constexpr std::array DISC_EXTENSIONS{"iso", "bin", "cue", "chd", "pbp", "cso", "m3u", "gdi", "img", "mdf"};

// A raw track/data file normally referenced by a separate cue/playlist sheet
inline constexpr std::array DISC_TRACK_EXTENSIONS{"bin", "img", "mdf"};

inline constexpr std::array DISC_SHEET_EXTENSIONS{"cue", "gdi", "m3u"};

// TODO
// Sheets that can name a track, which is a wider set than the sheets we can read as content: a
// .ccd is not a format anything here identifies, but a track it names is still spoken for and must
// not be catalogued in its own right
inline constexpr std::array TRACK_NAMING_EXTENSIONS{"cue", "gdi", "m3u", "ccd"};

[[nodiscard]] inline bool namesTracks(const std::string &extension) {
  return std::ranges::find(TRACK_NAMING_EXTENSIONS, extension) != TRACK_NAMING_EXTENSIONS.end();
}

// TODO
// The text after the last dot of a file's own name, lower-cased and without the dot. Takes the base
// name first, so a dot in a parent directory cannot be read as the extension
[[nodiscard]] inline std::string suffixOf(const std::string &path) {
  const auto separator = path.find_last_of("/\\");
  const auto name = separator == std::string::npos ? path : path.substr(separator + 1);
  const auto dot = name.find_last_of('.');

  if (dot == std::string::npos) {
    return {};
  }

  auto suffix = name.substr(dot + 1);
  std::ranges::transform(suffix, suffix.begin(), [](const unsigned char c) { return std::tolower(c); });
  return suffix;
}

// Any disc-image container or sheet extension the scanner should try to identify
[[nodiscard]] inline bool isDiscExtension(const std::string &extension) {
  return std::ranges::find(DISC_EXTENSIONS, extension) != DISC_EXTENSIONS.end();
}

// A raw track sitting beside the sheet that names it is reached through that sheet, so the
// track is not catalogued in its own right
[[nodiscard]] inline bool isDiscTrackExtension(const std::string &extension) {
  return std::ranges::find(DISC_TRACK_EXTENSIONS, extension) != DISC_TRACK_EXTENSIONS.end();
}

[[nodiscard]] inline bool isDiscSheetExtension(const std::string &extension) {
  return std::ranges::find(DISC_SHEET_EXTENSIONS, extension) != DISC_SHEET_EXTENSIONS.end();
}

} // namespace firelight::library

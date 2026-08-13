#pragma once

#include <algorithm>
#include <array>
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

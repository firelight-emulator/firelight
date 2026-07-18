#pragma once

#include <string>

namespace firelight::library {

// Disc-image extensions are ambiguous: the same extension is used by many
// consoles, so the platform can only be determined by inspecting the disc's
// contents (rcheevos AUTO detection). These classifiers are platform-independent
// file-type facts. Cartridge (rom) extensions map to a platform and are resolved
// via PlatformService::platformIdForExtension instead

// Any disc-image container or sheet extension the scanner should try to identify
[[nodiscard]] inline bool isDiscExtension(const std::string &extension) {
  return extension == "iso" || extension == "bin" || extension == "cue" ||
         extension == "chd" || extension == "pbp" || extension == "cso" ||
         extension == "m3u" || extension == "gdi" || extension == "ccd" ||
         extension == "img" || extension == "mdf" || extension == "nrg";
}

// A raw track/data file normally referenced by a separate cue/playlist sheet
// When such a sheet sits next to it we classify the sheet (not the raw track) to
// avoid duplicate library entries. iso/chd/pbp are self-contained (not tracks)
[[nodiscard]] inline bool isDiscTrackExtension(const std::string &extension) {
  return extension == "bin" || extension == "img" || extension == "mdf" ||
         extension == "nrg";
}

[[nodiscard]] inline bool isDiscSheetExtension(const std::string &extension) {
  return extension == "cue" || extension == "gdi" || extension == "ccd" ||
         extension == "m3u";
}

} // namespace firelight::library

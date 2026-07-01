#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct rc_hash_iterator;

namespace firelight::library {

// A member file of a multi-file disc set: a cue/gdi track or an .m3u-listed
// disc. `path` is the on-disk path (loose sets) or the entry name within the
// archive (in-archive sets).
struct IdentifiedDiscMember {
  std::string path;
  std::string role; // "track" or "disc"
};

// The platform + canonical content hash determined by inspecting disc contents.
struct DiscIdentity {
  bool valid = false;
  int platformId = -1; // PlatformMetadata::PLATFORM_ID_UNKNOWN
  std::string contentHash;
};

// Identifies disc images (loose or inside an archive) via rcheevos AUTO
// detection, and resolves a multi-file disc set's member files. This is the
// disc-specific counterpart that ContentIdentifier delegates to.
class DiscInspector {
public:
  // Inspects a loose disc image; for cue/gdi/m3u sheets, fills outMembers.
  DiscIdentity inspectFile(const std::string &path,
                           std::vector<IdentifiedDiscMember> &outMembers) const;

  // Inspects a disc image inside an archive, extracting the set to a temp
  // directory; fills outMembers with the set's other entries.
  DiscIdentity
  inspectArchiveEntry(const std::string &archivePath,
                      const std::string &entryName,
                      std::vector<IdentifiedDiscMember> &outMembers) const;

private:
  // Runs rcheevos AUTO detection over a disc image on disk.
  DiscIdentity detect(const std::string &discFilePath) const;

  // For a loose cue/gdi/m3u sheet, finds the sibling member files on disk.
  std::vector<IdentifiedDiscMember>
  collectLooseMembers(const std::string &sheetPath) const;

  // True when the iterator's first data track holds the Sega Saturn magic.
  static bool isSaturn(rc_hash_iterator &iterator);

  // Plausible sibling-filename tokens out of a cue/gdi/m3u sheet.
  static std::vector<std::string>
  sheetFilenameCandidates(const std::vector<uint8_t> &sheetBytes);

  static std::string roleForBaseName(const std::string &baseNameLower);
};

} // namespace firelight::library

// TODO: NEEDS REVIEW
#pragma once

#include <firelight/library/identify_outcome.hpp>

#include <cstdint>
#include <rcheevos/rc_consoles.h>
#include <string>
#include <vector>

struct rc_hash_iterator;

namespace firelight::platforms {
class IPlatformService;
}

namespace firelight::library {

// A member file of a multi-file disc set: a cue/gdi track or an .m3u-listed
// disc. `path` is the on-disk path (loose sets) or the entry name within the
// archive (in-archive sets)
struct IdentifiedDiscMember {
  std::string path;
  std::string role; // "track" or "disc"
};

// The platform + canonical content hash determined by inspecting disc contents
struct DiscIdentity {
  IdentifyOutcome outcome = IdentifyOutcome::NotRecognized;
  int platformId = -1; // PlatformService::PLATFORM_ID_UNKNOWN
  std::string contentHash;

  // TODO
  // What the format turned out to be, for a system with no platform to put it under. Keeping it
  // is the difference between telling somebody they own GameCube discs and telling them a file
  // could not be catalogued
  std::string identifiedAs;

  // TODO
  // Set when the bytes turned out to be a cartridge wearing a disc extension, which .bin
  // routinely is. The hash is left empty so the caller takes the ordinary cartridge path and
  // gets the same one the loader will compute at launch
  bool isCartridge = false;

  /** Whether this names a real platform and hash */
  [[nodiscard]] bool isIdentified() const { return outcome == IdentifyOutcome::Identified; }
};

// Identifies disc images (loose or inside an archive) via rcheevos AUTO
// detection, and resolves a multi-file disc set's member files. This is the
// disc-specific counterpart that ContentIdentifier delegates to
class DiscInspector {
public:
  // TODO
  // Consoles asked outright because rcheevos has no handler for a raw sector image. Every one has
  // to map to a platform, or a disc identifies here and then has nowhere to land
  static constexpr int RAW_SECTOR_CONSOLES[] = {RC_CONSOLE_PLAYSTATION, RC_CONSOLE_PLAYSTATION_2, RC_CONSOLE_3DO,
                                                RC_CONSOLE_SEGA_CD};

  explicit DiscInspector(platforms::IPlatformService &platformService);

  // Inspects a loose disc image; for cue/gdi/m3u sheets, fills outMembers
  DiscIdentity inspectFile(const std::string &path, std::vector<IdentifiedDiscMember> &outMembers) const;

  // Inspects a disc image inside an archive, extracting the set to a temp
  // directory; fills outMembers with the set's other entries
  DiscIdentity inspectArchiveEntry(const std::string &archivePath, const std::string &entryName,
                                   std::vector<IdentifiedDiscMember> &outMembers) const;

  // Plausible sibling-filename tokens out of a cue/gdi/m3u sheet
  [[nodiscard]] static std::vector<std::string> sheetFilenameCandidates(const std::vector<uint8_t> &sheetBytes);

  // TODO
  // The files a sheet names, resolved against the sheet's own directory. A playlist keeps a line
  // naming a file nobody has; a cue drops one, because there a line's fields are not all paths
  [[nodiscard]] std::vector<IdentifiedDiscMember> collectLooseMembers(const std::string &sheetPath) const;

private:
  // Runs rcheevos AUTO detection over a disc image on disk
  DiscIdentity detect(const std::string &discFilePath) const;

  // For a loose cue/gdi/m3u sheet, finds the sibling member files on disk

  // TODO
  // What the bytes say a file is once nothing has read it as a disc. Only ever asked after the
  // disc walk, because disc structure has to win over a cartridge header
  [[nodiscard]] DiscIdentity classifyByContent(const std::string &path, IdentifyOutcome discOutcome) const;

  // True when the iterator's first data track holds the Sega Saturn magic
  static bool isSaturn(rc_hash_iterator &iterator);

  // TODO
  // The discs a playlist names, one per line, with blanks and comments left out
  static std::vector<std::string> playlistLines(const std::vector<uint8_t> &bytes);

  static std::string roleForBaseName(const std::string &baseNameLower);

  platforms::IPlatformService &m_platformService;
};

} // namespace firelight::library

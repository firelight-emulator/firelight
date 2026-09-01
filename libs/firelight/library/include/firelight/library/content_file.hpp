// TODO: NEEDS REVIEW
#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace firelight::library {

// TODO
/**
 * What a recognised file is, which is what decides whether it stands for a game.
 *
 * Everything the walk recognises gets a row, so which files exist and what was decided about each
 * can be read back rather than inferred from what is missing
 */
enum class ContentRole {
  Dump = 0,    // A game's content: what is hashed, entered in the library and launched
  Track = 1,   // A raw track a sheet speaks for, reached through that sheet
  Playlist = 2 // A statement about which discs belong together, never a game of its own
};

enum class ContentType {
  Cartridge = 0, // Self-contained ROM
  Disc = 1,      // Single disc image or the primary file of a multi-file disc set
};

/**
 * A managed content file: a ROM or disc image that has been identified and catalogued
 */
struct ContentFile {
  int m_id = -1;
  ContentType m_type = ContentType::Cartridge;
  size_t m_fileSizeBytes = 0;
  std::string m_filePath;
  std::string m_fileMd5;
  std::string m_fileCrc32;
  bool m_inArchive = false;
  std::string m_archivePathName;
  int m_platformId = -1;
  std::string m_contentHash;

  int m_discNumber = 0; // 0 when not a disc, 1 for the first disc of a set, etc

  std::vector<std::string> m_regions;

  // TODO
  // The dump's own folded title, read from its filename at ingest. Grouping matches on this rather
  // than on the entry's, which is not written until metadata population has run
  std::string m_normalizedTitle;

  // TODO
  // The revision tag from the filename, empty when it carries none
  std::string m_revision;

  int m_contentDirectoryId = -1; // The content directory this file was found under

  ContentRole m_role = ContentRole::Dump;

  int64_t m_missingSince = 0; // When the file stopped being on disk, 0 while it is there
};

} // namespace firelight::library

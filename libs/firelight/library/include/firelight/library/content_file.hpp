#pragma once

#include <cstddef>
#include <string>

namespace firelight::library {

// What kind of content a file holds. Cartridge content is a single self-
// contained ROM; Disc content may be a single image or the primary file (a
// cue/gdi/m3u sheet) of a multi-file disc set whose members live in disc_members
enum class ContentType {
  Cartridge = 0,
  Disc = 1,
};

// A managed content file: a ROM or disc image that has been identified and
// catalogued. The launchable identity is m_contentHash; m_filePath/m_inArchive
// locate the bytes on disk
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
  // The content directory (content_directoriesv1.id) this file was found under,
  // resolved by longest path-prefix match. -1 when it belongs to no known
  // directory (e.g. imported before source tracking, and unmatched by the
  // backfill). This is the folder source for smart folders
  int m_contentDirectoryId = -1;
};

} // namespace firelight::library

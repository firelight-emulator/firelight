#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace firelight::library {

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

  std::optional<int> m_discSetId{}; // The multi-disc game this file belongs to
  int m_discNumber = 0;             // 0 when not a disc, 1 for the first disc of a set, etc
  bool m_discNumberUserSet = false; // Indicates that disc number was set by user so don't overwrite it

  std::vector<std::string> m_regions;

  int m_gameId = 0; // TODO: Not sure if I want to keep

  int m_contentDirectoryId = -1; // The content directory this file was found under

  int64_t m_missingSince = 0; // When the file stopped being on disk, 0 while it is there
};

} // namespace firelight::library

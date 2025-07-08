#pragma once

namespace firelight::library {
struct RomFileInfo {
  int m_id = -1;
  size_t m_fileSizeBytes;
  std::string m_filePath;
  std::string m_fileMd5;
  std::string m_fileCrc32;
  bool m_inArchive = false;
  std::string m_archivePathName;
  int m_platformId;
  std::string m_contentHash;
};
} // namespace firelight::library
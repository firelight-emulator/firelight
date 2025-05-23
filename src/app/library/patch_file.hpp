#pragma once
#include "../patching/bps_patch.hpp"
#include "../patching/ips_patch.hpp"
#include "../patching/rom_patch.hpp"
#include "../patching/ups_patch.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace firelight::library {
class PatchFile {
public:
  enum PatchType { UNKNOWN, IPS, BPS, UPS, XDELTA };
  ~PatchFile() = default;
  PatchFile() = default;
  int m_id = 0;
  std::string m_filePath;
  int64_t m_fileSize = 0;
  std::string m_fileMd5;
  std::string m_fileCrc32;
  std::string m_targetFileMd5;
  std::string m_patchedMd5;
  std::string m_patchedContentHash;
  bool m_inArchive = false;
  std::string m_archiveFilePath;

  bool load();

  [[nodiscard]] PatchType getType() const;

  bool isValid();
  [[nodiscard]] std::vector<uint8_t>
  patch(const std::vector<uint8_t> &data) const;

private:
  bool m_valid = false;
  bool m_loaded = false;
  bool m_fileExists = false;
  PatchType m_patchType = UNKNOWN;
  std::shared_ptr<patching::IRomPatch> m_patch{};

  std::vector<uint8_t> m_patchData;
};
} // namespace firelight::library
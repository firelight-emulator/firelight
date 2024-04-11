#pragma once
#include "rom_patch.hpp"

#include <stdexcept>
#include <string>

namespace firelight::patching {

struct UPSPatchRecord {
  // Relative offset from the previous record
  int64_t offset = 0;

  // XOR data to apply at the offset
  std::vector<uint8_t> data;
};

class UPSPatch final : public IRomPatch {
public:
  explicit UPSPatch(const std::string &path);
  explicit UPSPatch(const std::vector<uint8_t> &data);
  [[nodiscard]] std::vector<uint8_t>
  patchRom(const std::vector<uint8_t> &data) const override;
  [[nodiscard]] std::vector<UPSPatchRecord> getRecords() const;

  [[nodiscard]] uint32_t getInputFileSize() const { return inputFileSize; }
  [[nodiscard]] uint32_t getOutputFileSize() const { return outputFileSize; }
  [[nodiscard]] uint32_t getInputFileCRC32Checksum() const {
    return inputFileCRC32Checksum;
  }
  [[nodiscard]] uint32_t getOutputFileCRC32Checksum() const {
    return outputFileCRC32Checksum;
  }
  [[nodiscard]] uint32_t getPatchFileCRC32Checksum() const {
    return patchFileCRC32Checksum;
  }
  [[nodiscard]] bool isValid() const override;

private:
  bool m_isValid = true;
  uint32_t inputFileSize = 0;
  uint32_t outputFileSize = 0;
  uint32_t inputFileCRC32Checksum = 0;
  uint32_t outputFileCRC32Checksum = 0;
  uint32_t patchFileCRC32Checksum = 0;

  std::vector<UPSPatchRecord> records;
};

} // namespace firelight::patching

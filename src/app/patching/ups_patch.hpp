#pragma once
#include "rom_patch.hpp"

#include <stdexcept>
#include <string>

namespace firelight::patching {

struct UPSPatchRecord {
  // Relative offset from the previous record
  uint64_t offset = 0;

  // XOR data to apply at the offset
  std::vector<uint8_t> data;
};

class UPSPatch final : public IRomPatch {
public:
  explicit UPSPatch(const std::vector<uint8_t> &data);
  [[nodiscard]] std::vector<uint8_t>
  patchRom(const std::vector<uint8_t> &data) const override;
  std::vector<UPSPatchRecord> getRecords();

  uint64_t readVLV(const std::vector<uint8_t> &data, size_t &index) {
    uint64_t result = 0;
    uint32_t shift = 1;
    while (true) {
      if (index >= data.size()) {
        throw std::runtime_error("Can't read UPS VLV at offset " +
                                 std::to_string(index));
      }

      const uint8_t x = data[index++];
      result += (x & 0x7F) * shift;
      if ((x & 0x80) != 0) {
        break;
      }
      shift <<= 7;
      result += shift;
    }

    return result;
  }

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

private:
  uint32_t inputFileSize = 0;
  uint32_t outputFileSize = 0;
  uint32_t inputFileCRC32Checksum = 0;
  uint32_t outputFileCRC32Checksum = 0;
  uint32_t patchFileCRC32Checksum = 0;

  std::vector<UPSPatchRecord> records;
};

} // namespace firelight::patching

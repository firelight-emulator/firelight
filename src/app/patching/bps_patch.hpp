#pragma once
#include "rom_patch.hpp"

#include <stdexcept>
#include <string>

namespace firelight::patching {

struct BPSPatchAction {
  // Relative offset from the previous record
  int64_t type;
  int64_t length;
  std::vector<uint8_t> bytes;
  int64_t relativeOffset;
};

class BPSPatch final : public IRomPatch {
public:
  explicit BPSPatch(const std::vector<uint8_t> &data);
  [[nodiscard]] std::vector<uint8_t>
  patchRom(const std::vector<uint8_t> &data) const override;

  int64_t readVLV(const std::vector<uint8_t> &data, size_t &index) {
    int64_t result = 0;
    uint32_t shift = 1;
    while (true) {
      if (index >= data.size()) {
        throw std::runtime_error("Can't read UPS VLV at offset " +
                                 std::to_string(index));
      }

      uint8_t x = data[index++];
      result += (x & 0x7F) * shift;
      if ((x & 0x80) != 0) {
        break;
      }
      shift <<= 7;
      result += shift;
    }

    return result;
  }

  [[nodiscard]] uint32_t getInputFileSize() const { return m_inputFileSize; }
  [[nodiscard]] uint32_t getOutputFileSize() const { return m_outputFileSize; }
  [[nodiscard]] uint32_t getInputFileCRC32Checksum() const {
    return m_inputFileCRC32Checksum;
  }
  [[nodiscard]] uint32_t getOutputFileCRC32Checksum() const {
    return m_outputFileCRC32Checksum;
  }
  [[nodiscard]] uint32_t getPatchFileCRC32Checksum() const {
    return m_patchFileCRC32Checksum;
  }

  std::vector<BPSPatchAction> getActions() { return m_actions; }
  std::string getMetadata() { return m_metadata; }

private:
  uint32_t m_inputFileSize = 0;
  uint32_t m_outputFileSize = 0;
  std::string m_metadata;
  uint32_t m_inputFileCRC32Checksum = 0;
  uint32_t m_outputFileCRC32Checksum = 0;
  uint32_t m_patchFileCRC32Checksum = 0;

  void copyToFile(std::vector<uint8_t> &srcFile, std::vector<uint8_t> &destFile,
                  size_t &destOffset, size_t length) {
    destFile.insert(destFile.end(), srcFile.begin() + destOffset,
                    srcFile.begin() + destOffset + length);
    destOffset += length;
  }

  void writeBytes(std::vector<uint8_t> &destFile,
                  const std::vector<uint8_t> &bytes) {
    destFile.insert(destFile.end(), bytes.begin(), bytes.end());
  }

  void sourceCopy(std::vector<uint8_t> &srcFile, std::vector<uint8_t> &destFile,
                  size_t &srcOffset, size_t &destOffset, size_t length) {
    for (size_t i = 0; i < length; ++i) {
      destFile.push_back(srcFile[srcOffset]);
      ++srcOffset;
      ++destOffset;
    }
  }

  void targetCopy(std::vector<uint8_t> &destFile, size_t &destOffset,
                  size_t &srcOffset, size_t length) {
    for (size_t i = 0; i < length; ++i) {
      destFile.push_back(destFile[destOffset]);
      ++destOffset;
    }
  }

  std::vector<BPSPatchAction> m_actions;
};

} // namespace firelight::patching
#pragma once
#include "rom_patch.hpp"

#include <stdexcept>
#include <string>

namespace firelight::patching {

struct BPSPatchAction {
  int64_t type;
  int64_t length;
  std::vector<uint8_t> bytes;
  int64_t relativeOffset;
};

class BPSPatch final : public IRomPatch {
public:
  explicit BPSPatch(const std::string &path);
  explicit BPSPatch(const std::vector<uint8_t> &data);
  [[nodiscard]] std::vector<uint8_t>
  patchRom(const std::vector<uint8_t> &data) const override;

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

  [[nodiscard]] std::vector<BPSPatchAction> getActions() const {
    return m_actions;
  }
  [[nodiscard]] std::string getMetadata() const { return m_metadata; }
  [[nodiscard]] bool isValid() const override;

private:
  bool m_isValid = true;

  uint32_t m_inputFileSize = 0;
  uint32_t m_outputFileSize = 0;
  std::string m_metadata;
  uint32_t m_inputFileCRC32Checksum = 0;
  uint32_t m_outputFileCRC32Checksum = 0;
  uint32_t m_patchFileCRC32Checksum = 0;

  std::vector<BPSPatchAction> m_actions;
};

} // namespace firelight::patching
#pragma once

#include "rom_patch.hpp"
#include <cstdint>
#include <vector>

namespace firelight::patching {

// https://zerosoft.zophar.net/ips.php
struct IPSPatchRecord {
  uint32_t offset;
  std::vector<uint8_t> data;

  // Some patches are RLE encoded, but otherwise we just write the data once
  uint16_t numTimesToWrite = 1;
};

class IPSPatch final : public IRomPatch {
public:
  explicit IPSPatch(std::vector<uint8_t> &data);
  [[nodiscard]] std::vector<uint8_t>
  patchRom(const std::vector<uint8_t> &data) const override;
  std::vector<IPSPatchRecord> getRecords();

private:
  std::vector<IPSPatchRecord> records;
};

} // namespace firelight::patching

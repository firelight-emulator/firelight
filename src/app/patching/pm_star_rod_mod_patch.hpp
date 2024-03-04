#pragma once

#include "rom_patch.hpp"
#include <cstdint>
#include <vector>

namespace firelight::patching {
struct PMStarRodPatchRecord {
  uint32_t offset;
  std::vector<uint8_t> data;
};

class PMStarRodModPatch final : public IRomPatch {
public:
  explicit PMStarRodModPatch(const std::vector<uint8_t> &data);
  std::vector<uint8_t>
  patchRom(const std::vector<uint8_t> &data) const override;
  std::vector<PMStarRodPatchRecord> getRecords();

private:
  std::vector<PMStarRodPatchRecord> records;
};
} // namespace firelight::patching

//
// Created by alexs on 11/25/2023.
//

#ifndef FIRELIGHT_PM_STAR_ROD_MOD_PATCH_HPP
#define FIRELIGHT_PM_STAR_ROD_MOD_PATCH_HPP

#include "rom_patch.hpp"

#include <cstdint>
#include <vector>

namespace FL::Patching {
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
} // namespace FL::Patching

#endif // FIRELIGHT_PM_STAR_ROD_MOD_PATCH_HPP

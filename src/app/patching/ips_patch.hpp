//
// Created by alexs on 1/11/2024.
//

#ifndef IPS_PATCH_HPP
#define IPS_PATCH_HPP
#include <cstdint>
#include <vector>

namespace Firelight::Patching {

// https://zerosoft.zophar.net/ips.php
struct IPSPatchRecord {
  uint32_t offset;
  std::vector<uint8_t> data;

  // Some patches are RLE encoded
  uint16_t numTimesToWrite = 0;
};

class IPSPatch {
public:
  explicit IPSPatch(const std::vector<uint8_t> &data);
  std::vector<uint8_t> patchRom(std::vector<uint8_t> &data);
  std::vector<IPSPatchRecord> getRecords();

private:
  std::vector<IPSPatchRecord> records;
};

} // namespace Firelight::Patching
// Firelight

#endif // IPS_PATCH_HPP

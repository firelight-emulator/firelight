#include "ups_patch.hpp"

#include <iostream>

namespace firelight::patching {
UPSPatch::UPSPatch(const std::vector<uint8_t> &data) {
  size_t index = 4; // Skip UPS1 magic string

  inputFileSize = readVLV(data, index);
  outputFileSize = readVLV(data, index);

  while (index < data.size() - 12) {
    auto relativeOffset = readVLV(data, index);

    std::vector<uint8_t> XORdifferences;
    uint8_t byte;
    while ((byte = data[index++]) != 0) {
      XORdifferences.push_back(byte);
    }

    records.emplace_back(UPSPatchRecord{relativeOffset, XORdifferences});
  }

  inputFileCRC32Checksum = data[index + 3] << 24 | data[index + 2] << 16 |
                           data[index + 1] << 8 | data[index];
  index += 4;
  outputFileCRC32Checksum = data[index + 3] << 24 | data[index + 2] << 16 |
                            data[index + 1] << 8 | data[index];
  index += 4;
  patchFileCRC32Checksum = data[index + 3] << 24 | data[index + 2] << 16 |
                           data[index + 1] << 8 | data[index];
}
std::vector<uint8_t>
UPSPatch::patchRom(const std::vector<uint8_t> &data) const {
  std::vector<uint8_t> patchedRom(outputFileSize);

  std::copy(data.begin(), data.end(), patchedRom.begin());

  auto romCursor = data.begin();
  auto patchedRomCursor = patchedRom.begin();

  for (const auto &record : records) {
    romCursor += record.offset;
    patchedRomCursor += record.offset;

    for (const uint8_t i : record.data) {
      *patchedRomCursor = (romCursor > data.end() ? 0x00 : *romCursor) ^ i;

      ++patchedRomCursor;
      ++romCursor;
    }

    ++patchedRomCursor;
    ++romCursor;
  }

  return patchedRom;
}
std::vector<UPSPatchRecord> UPSPatch::getRecords() { return records; }
} // namespace firelight::patching
#include "ups_patch.hpp"
#include "util.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <zlib.h>

namespace firelight::patching {
UPSPatch::UPSPatch(const std::string &path) {
  const auto size = std::filesystem::file_size(path);

  std::ifstream file(path, std::ios::binary);

  std::vector<uint8_t> data(size);
  file.read(reinterpret_cast<char *>(data.data()), size);

  file.close();

  *this = UPSPatch(data);
}

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

  auto crc = crc32(0L, nullptr, 0);
  crc = crc32(crc, data.data(), data.size() - 4);
  if (crc != patchFileCRC32Checksum) {
    printf("Patch data does not match expected checksum");
    m_isValid = false;
  }
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
std::vector<UPSPatchRecord> UPSPatch::getRecords() const { return records; }
bool UPSPatch::isValid() const { return m_isValid; }
} // namespace firelight::patching
//
// Created by alexs on 1/11/2024.
//

#include "ips_patch.hpp"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <spdlog/spdlog.h>

namespace firelight::Patching {

void printHex(const std::vector<uint8_t> &data) {
  for (auto byte : data) {
    std::cout << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<int>(byte) << " ";
  }
  std::cout << std::dec << std::endl; // Reset back to decimal format
}

IPSPatch::IPSPatch(std::vector<uint8_t> &data) {
  auto cursor = data.data();
  // Skip PATCH header
  cursor += 5;
  int numBytesRead = 5;

  while (numBytesRead < data.size()) {
    IPSPatchRecord record{};

    const uint32_t offset = cursor[0] << 16 | cursor[1] << 8 | cursor[2];
    // spdlog::info("offset: {}", offset);
    cursor += 3;
    numBytesRead += 3;

    // If we hit the EOF marker, we're done
    if (offset == 0x454F46) {
      break;
    }

    record.offset = offset;
    const uint16_t size = cursor[0] << 8 | cursor[1];
    // spdlog::info("size: {}", size);
    cursor += 2;
    numBytesRead += 2;

    if (size == 0) {
      const auto rleSize = cursor[0] << 8 | cursor[1];
      const uint8_t byte = cursor[2];

      record.numTimesToWrite = rleSize;
      record.data.resize(1);
      record.data.at(0) = byte;

      cursor += 3;
      numBytesRead += 3;
    } else {
      record.data.resize(size);
      memcpy(record.data.data(), cursor, size);
      cursor += size;
      numBytesRead += size;
    }

    records.emplace_back(record);
  }

  spdlog::info("Number of bytes read: {}", numBytesRead);
}
std::vector<uint8_t>
IPSPatch::patchRom(const std::vector<uint8_t> &data) const {
  const auto last = records[records.size() - 1];

  const auto totalLen = last.offset + (last.data.size() * last.numTimesToWrite);

  auto size = data.size();
  if (totalLen > data.size()) {
    size = totalLen;
  }

  std::vector<uint8_t> result(size);
  memcpy(result.data(), data.data(), data.size());

  const auto begin = result.data();
  auto cursor = begin;
  for (const auto &record : records) {
    cursor = begin + record.offset;

    std::vector<uint8_t> dataToWrite;

    for (int i = 0; i < record.numTimesToWrite; ++i) {
      dataToWrite.insert(dataToWrite.end(), record.data.begin(),
                         record.data.end());
    }
    memcpy(cursor, dataToWrite.data(), dataToWrite.size());

    dataToWrite.clear();
  }

  return result;
}

std::vector<IPSPatchRecord> IPSPatch::getRecords() { return records; }
} // namespace Firelight::Patching
  // Firelight
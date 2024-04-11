#pragma once
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace firelight::patching {
static int64_t readVLV(const std::vector<uint8_t> &data, size_t &index) {
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
} // namespace firelight::patching
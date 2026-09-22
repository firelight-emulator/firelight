#pragma once

#include <cstdint>
#include <vector>

namespace firelight::patching {

class Yay0Codec {
public:
  static std::vector<uint8_t> decompress(const uint8_t *data);
};

} // namespace firelight::patching

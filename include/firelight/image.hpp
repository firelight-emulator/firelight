#pragma once

#include <cstdint>
#include <vector>

namespace firelight {

// A PNG-encoded image
struct Image {
  std::vector<uint8_t> pngData;

  [[nodiscard]] bool isNull() const { return pngData.empty(); }
};

} // namespace firelight

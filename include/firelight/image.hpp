#pragma once

#include <cstdint>
#include <vector>

namespace firelight {

// A PNG-encoded image. We use this instead of QImage so we can keep Qt out of the modules
struct Image {
  std::vector<uint8_t> pngData;

  [[nodiscard]] bool isNull() const { return pngData.empty(); }
};

} // namespace firelight

#pragma once

#include <cstdint>
#include <vector>

namespace firelight {

// One emulator frame as raw pixels, normalized to tightly packed RGBA8888 the right way up
struct VideoFrame {
  std::vector<uint8_t> pixels;
  int width = 0;
  int height = 0;

  // Assigned by the slot that published it, increases each frame
  uint64_t id = 0;

  [[nodiscard]] bool isNull() const { return pixels.empty() || width <= 0 || height <= 0; }
};

} // namespace firelight

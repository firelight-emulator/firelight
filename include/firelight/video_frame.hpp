#pragma once

#include <cstdint>
#include <vector>

namespace firelight {

// One emulator frame as raw pixels, normalized to tightly packed RGBA8888 the right way up. Qt-free,
// so the emulator can produce frames with no renderer attached (see firelight::Image for the
// PNG-encoded form the stored artifacts use)
struct VideoFrame {
  std::vector<uint8_t> pixels;
  int width = 0;
  int height = 0;

  // Assigned by the slot that published it. Strictly increasing, so a consumer can tell whether what
  // it is holding is older than what it already showed, and how many frames it never saw
  uint64_t id = 0;

  [[nodiscard]] bool isNull() const { return pixels.empty() || width <= 0 || height <= 0; }
};

} // namespace firelight

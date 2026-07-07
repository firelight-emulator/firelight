#pragma once

#include <cstdint>
#include <vector>

namespace firelight {

// A PNG-encoded image. Qt-free so domain types (e.g. SuspendPoint) don't force
// Qt6::Gui on their consumers; the GUI layer converts to/from QImage at the
// boundary (see src/gui/image_qt.hpp).
struct Image {
  std::vector<uint8_t> pngData;

  [[nodiscard]] bool isNull() const { return pngData.empty(); }
};

} // namespace firelight

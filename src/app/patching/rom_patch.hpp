#pragma once

#include <cstdint>
#include <vector>

namespace firelight::patching {

class IRomPatch {
public:
  virtual ~IRomPatch() = default;
  [[nodiscard]] virtual std::vector<uint8_t>
  patchRom(const std::vector<uint8_t> &data) const = 0;

  virtual bool isValid() const = 0;
};

} // namespace firelight::patching

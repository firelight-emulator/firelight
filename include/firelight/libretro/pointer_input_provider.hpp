#pragma once

#include <utility>

namespace firelight::libretro {
  class IPointerInputProvider {
  public:
    virtual ~IPointerInputProvider() = default;

    [[nodiscard]] virtual std::pair<int16_t, int16_t> getPointerPosition() const = 0;

    [[nodiscard]] virtual bool isPressed() const = 0;
  };
} // namespace firelight::libretro

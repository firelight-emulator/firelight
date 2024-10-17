#pragma once

#include "retropad.hpp"
#include <optional>

namespace firelight::libretro {
  class IPointerInputProvider {
  public:
    virtual ~IPointerInputProvider() = default;

    virtual std::pair<int16_t, int16_t> getPointerPosition() const = 0;

    virtual bool isPressed() const = 0;
  };
} // namespace firelight::libretro

#pragma once

#include "retropad.hpp"

#include <memory>

namespace firelight::libretro {
class IRetropadProvider {
public:
  virtual ~IRetropadProvider() = default;

  virtual std::shared_ptr<IRetroPad> getRetropadForPlayerIndex(int t_player) = 0;
};
} // namespace firelight::libretro

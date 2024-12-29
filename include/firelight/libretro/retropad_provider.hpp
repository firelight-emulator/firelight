#pragma once

#include "retropad.hpp"
#include <optional>

namespace firelight::libretro {
  class IRetropadProvider {
  public:
    virtual ~IRetropadProvider() = default;

    virtual std::optional<IRetroPad *> getRetropadForPlayerIndex(int t_player, int platformId) = 0;
  };
} // namespace firelight::libretro

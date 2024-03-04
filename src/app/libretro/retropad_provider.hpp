#pragma once

#include <optional>

namespace firelight::libretro {
class IRetroPad;

class IRetropadProvider {
public:
  virtual ~IRetropadProvider() = default;
  virtual std::optional<IRetroPad *> getRetropadForPlayer(int t_player) = 0;
};

} // namespace firelight::libretro

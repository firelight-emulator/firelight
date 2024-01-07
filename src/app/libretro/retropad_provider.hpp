//
// Created by alexs on 1/6/2024.
//

#ifndef I_RETROPAD_PROVIDER_HPP
#define I_RETROPAD_PROVIDER_HPP
#include <optional>

namespace libretro {
class IRetroPad;

class IRetropadProvider {
public:
  virtual ~IRetropadProvider() = default;
  virtual std::optional<IRetroPad *> getRetropadForPlayer(int t_player) = 0;
};

} // namespace libretro

#endif // I_RETROPAD_PROVIDER_HPP

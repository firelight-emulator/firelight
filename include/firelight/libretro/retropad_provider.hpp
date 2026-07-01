#pragma once

#include "retropad.hpp"
#include <memory>

namespace firelight::libretro {
class IRetropadProvider {
public:
  virtual ~IRetropadProvider() = default;

  // Returns the RetroPad bound to a player slot, or nullptr if the slot is
  // empty. The shared_ptr keeps the device alive for as long as the caller
  // holds it, so it is safe to use even if the device is unplugged mid-use.
  virtual std::shared_ptr<IRetroPad>
  getRetropadForPlayerIndex(int t_player) = 0;
};
} // namespace firelight::libretro

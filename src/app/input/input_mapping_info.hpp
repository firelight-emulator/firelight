#pragma once
#include <firelight/libretro/retropad_provider.hpp>

namespace firelight::input {
struct InputMappingInfo {
  int id = -1;
  libretro::IRetroPad::Input originalInput;
  libretro::IRetroPad::Input mappedInput;
};
} // namespace firelight::input
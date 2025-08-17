#pragma once
#include <firelight/libretro/retropad.hpp>
#include <string>

namespace firelight::input {
struct PlatformInputDescriptor {
  std::string displayName;
  libretro::IRetroPad::Input input;
};
} // namespace firelight::input
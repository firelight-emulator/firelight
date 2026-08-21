#pragma once

namespace firelight::libretro {
class ISystemInfoProvider {
public:
  virtual ~ISystemInfoProvider() = default;
  // TODO: Device power state, battery level
};
} // namespace firelight::libretro

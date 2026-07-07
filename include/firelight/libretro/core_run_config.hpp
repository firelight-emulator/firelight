#pragma once

#include "firelight/libretro/configuration_provider.hpp"

#include <memory>
#include <string>

namespace firelight::libretro {

// Everything a core needs at construction. Notably it must carry everything the
// core can query during retro_set_environment, which runs inside the Core
// constructor — so directories belong here, not in post-construction setters.
struct CoreRunConfig {
  int platformId = -1;
  std::string corePath;
  std::shared_ptr<IConfigurationProvider> configProvider;
  std::string systemDirectory;
  std::string saveDirectory;
};

} // namespace firelight::libretro

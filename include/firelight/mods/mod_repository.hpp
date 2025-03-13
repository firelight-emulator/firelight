#pragma once
#include "mod_info.hpp"

#include <optional>

namespace firelight::mods {
class IModRepository {
public:
  virtual ~IModRepository() = default;
  virtual std::optional<ModInfo> getModInfo(int modId) = 0;
};
}
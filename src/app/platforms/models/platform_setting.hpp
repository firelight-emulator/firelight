#pragma once
#include <string>

namespace firelight::platforms {

enum PlatformSettingType {
  TOGGLE,
  TEXT,
  DROPDOWN,
};

struct CoreOption {
  std::string key;
  std::string name;
  std::string description;
  std::string category;
  PlatformSettingType type;
  std::string defaultValue;
};

} // namespace firelight::platforms
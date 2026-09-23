#pragma once

#include <string>
#include <vector>

namespace firelight::settings {

// One selectable value of a raw libretro core option
struct CoreOptionValue {
  std::string value;
  std::string label;
};

// A raw libretro core option as the core declares it
struct CoreOption {
  std::string key;
  std::string label;
  std::string description;
  std::string defaultValue;
  std::vector<CoreOptionValue> values;
  std::string category;      // category key (empty is uncategorized/top level)
  std::string categoryLabel; // human-readable category name
};

} // namespace firelight::settings

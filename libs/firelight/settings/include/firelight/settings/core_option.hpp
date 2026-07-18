#pragma once

#include <string>
#include <vector>

namespace firelight::settings {

// One selectable value of a raw libretro core option
struct CoreOptionValue {
  std::string value;
  std::string label;
};

// A raw libretro core option as the core declares it (SET_CORE_OPTIONS_V2)
// Cached per core so the advanced options editor can render before a core has
// been launched in the current session
struct CoreOption {
  std::string key;
  std::string label;
  std::string description;
  std::string defaultValue;
  std::vector<CoreOptionValue> values;
  // Core-options v2 category grouping ("" = top-level/uncategorized)
  std::string category;      // category key
  std::string categoryLabel; // human-readable category name
};

} // namespace firelight::settings

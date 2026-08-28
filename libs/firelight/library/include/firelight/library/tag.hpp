#pragma once

#include <cstdint>
#include <string>

namespace firelight::library {

/**
 * A label the user puts on entries
 */
struct Tag {
  int id = -1;
  std::string name;
  std::string color;
  uint64_t createdAt = 0;

  int usageCount = 0; // This field is calculated, not stored
};

} // namespace firelight::library

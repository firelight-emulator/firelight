#pragma once

#include <cstdint>
#include <string>

namespace firelight::library {

/**
 * A label the user puts on entries.
 *
 * Its own table rather than a field on the entry, because a tag is managed — renamed, merged,
 * deleted, counted — and each of those is one statement here against a rewrite of every entry
 * carrying it
 */
struct Tag {
  int id = -1;
  std::string name;
  std::string color;
  uint64_t createdAt = 0;

  /** How many entries carry it, filled in by listing rather than stored */
  int usageCount = 0;
};

} // namespace firelight::library

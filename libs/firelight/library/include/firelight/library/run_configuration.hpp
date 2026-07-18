#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace firelight::library {
struct RunConfiguration {
  static constexpr std::string_view TYPE_ROM = "rom";
  static constexpr std::string_view TYPE_PATCH = "patch";

  int id = -1;
  std::string type;
  std::string contentHash;
  int contentFileId = -1;
  int patchId = -1;
  int64_t createdAt = 0;
};
} // namespace firelight::library

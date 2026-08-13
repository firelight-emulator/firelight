#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace firelight::library {
struct RunConfiguration {
  static constexpr std::string_view TYPE_ROM = "rom";
  static constexpr std::string_view TYPE_PATCH = "patch";
  static constexpr std::string_view TYPE_PLAYLIST = "playlist";

  int id = -1;
  std::string type;
  std::string contentHash;
  int contentFileId = -1;
  int patchId = -1;

  // TODO
  // The set this launches, unset when it launches a single file. A set's way in exists for as
  // long as the set does, rather than for as long as a file we generated survives on disk
  std::optional<int> discSetId{};

  int64_t createdAt = 0;
};
} // namespace firelight::library

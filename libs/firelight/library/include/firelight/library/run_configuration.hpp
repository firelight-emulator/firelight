#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace firelight::library {

/**
 * Represents a way to launch an entry: a ROM/disc file, optionally with a patch applied, or a playlist for a multi-disc
 * game
 */
struct RunConfiguration {
  static constexpr std::string_view TYPE_ROM = "rom";
  static constexpr std::string_view TYPE_PATCH = "patch";
  static constexpr std::string_view TYPE_PLAYLIST = "playlist";

  int id = -1;
  std::string type;
  std::string contentHash;
  int contentFileId = -1;
  int patchId = -1;
  std::optional<int> discSetId{};

  int64_t createdAt = 0;
};
} // namespace firelight::library

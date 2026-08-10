#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace firelight::library {
struct RunConfiguration {
  static constexpr std::string_view TYPE_ROM = "rom";
  static constexpr std::string_view TYPE_PATCH = "patch";
  // TODO
  // The playlist addressing every disc of a set. It stands for the whole game, so it beats a
  // single disc's own way in rather than tying with it
  static constexpr std::string_view TYPE_PLAYLIST = "playlist";

  int id = -1;
  std::string type;
  std::string contentHash;
  int contentFileId = -1;
  int patchId = -1;
  int64_t createdAt = 0;
};
} // namespace firelight::library

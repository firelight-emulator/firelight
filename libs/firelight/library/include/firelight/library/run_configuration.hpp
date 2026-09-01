// TODO: NEEDS REVIEW
#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace firelight::library {

// TODO
/**
 * A way to launch an entry.
 *
 * What kind it is, is what it points at: a disc set launches through the set's playlist, anything
 * else through the content file named here. A patch id means the bytes handed to the core are the
 * patched ones
 */
struct RunConfiguration {
  int id = -1;
  std::string contentHash;
  int contentFileId = -1;
  int patchId = -1;
  std::optional<int> discSetId{};

  // TODO
  // Which of an entry's ways in it launches through. Exactly one carries this, so nothing has to
  // score them against each other
  bool isDefault = false;

  int64_t createdAt = 0;
};
} // namespace firelight::library

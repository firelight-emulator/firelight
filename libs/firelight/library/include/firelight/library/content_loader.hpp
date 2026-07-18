#pragma once

#include <firelight/library/content_file.hpp>
#include <firelight/library/patch_file.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace firelight::library {

// The bytes to hand to the core for a piece of content, plus the canonical
// content hash they correspond to
struct LoadedContent {
  bool valid = false;
  std::vector<uint8_t> contentBytes;
  std::string contentHash;
};

// Loads content for play: reads a ROM (on disk or from an archive) and produces
// the normalized content bytes the core expects. The launch-time counterpart to
// ContentIdentifier
class ContentLoader {
public:
  [[nodiscard]] LoadedContent load(const ContentFile &info) const;

  // Applies a patch to already-loaded content, recomputing the bytes and hash
  void applyPatch(LoadedContent &content, int platformId,
                  const PatchFile &patch) const;
};

} // namespace firelight::library

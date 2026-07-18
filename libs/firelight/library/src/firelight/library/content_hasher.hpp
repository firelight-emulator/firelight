#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace firelight::library {

// The launch-ready content bytes and the canonical content hash derived from a
// file's raw bytes. For cartridge systems the bytes may be header-stripped or
// byte-swapped relative to the file on disk
struct HashedContent {
  std::vector<uint8_t> contentBytes;
  std::string contentHash;
};

// Computes the content bytes and canonical (RetroAchievements-compatible)
// content hash for a non-disc ROM, applying the platform-specific normalization:
// iNES/SNES header stripping, N64 byte-order swap, and the rcheevos buffer hash
// for everything else
class ContentHasher {
public:
  [[nodiscard]] HashedContent hash(int platformId, const std::vector<uint8_t> &fileBytes) const;

  // Lowercase-hex MD5 of a byte range (used for whole-file hashes)
  [[nodiscard]] static std::string md5(const uint8_t *data, size_t len);
};

} // namespace firelight::library

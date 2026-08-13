#include "firelight/library/rc_hash_logging.hpp"

#include <firelight/library/content_hasher.hpp>
#include <firelight/platforms/platform_service.hpp>

#include <algorithm>
#include <cstring>
#include <md5.h>
#include <rcheevos/rc_hash.h>

namespace firelight::library {
namespace {

std::string toHex(const uint8_t *data, size_t len) {
  static const char *digits = "0123456789abcdef";
  std::string out;
  out.reserve(len * 2);
  for (size_t i = 0; i < len; ++i) {
    out += digits[data[i] >> 4];
    out += digits[data[i] & 0x0F];
  }
  return out;
}

bool startsWith(const std::vector<uint8_t> &bytes, const uint8_t *signature, size_t n) {
  return bytes.size() >= n && std::memcmp(bytes.data(), signature, n) == 0;
}

//****************
// mega drive containers
//****************

// Where a Mega Drive cartridge carries its "SEGA" marker
constexpr size_t SEGA_MAGIC_OFFSET = 0x100;
constexpr size_t COPIER_HEADER_SIZE = 512;
constexpr size_t INTERLEAVE_BLOCK_SIZE = 0x4000;

bool hasSegaMagicAt(const std::vector<uint8_t> &bytes, const size_t offset) {
  return bytes.size() >= offset + 4 && std::memcmp(bytes.data() + offset, "SEGA", 4) == 0;
}

// TODO
// The same test Genesis Plus GX applies, so the two agree on what a file is: no marker where a
// plain cartridge would have one, and a size that is an odd number of 512-byte blocks. Detected
// from the bytes rather than the extension, so a dump that was renamed is still read correctly
bool isCopierImage(const std::vector<uint8_t> &bytes) {
  return !hasSegaMagicAt(bytes, SEGA_MAGIC_OFFSET) && bytes.size() % COPIER_HEADER_SIZE == 0 &&
         (bytes.size() / COPIER_HEADER_SIZE) % 2 == 1;
}

// TODO
// Drops the copier header, then un-scatters each 16KB block: the first half holds the odd bytes
// and the second half the even ones. Yields the plain cartridge dump, so the same game carries
// one identity whichever container it arrived in
std::vector<uint8_t> deinterleaveCopierImage(const std::vector<uint8_t> &bytes) {
  std::vector<uint8_t> rom(bytes.begin() + COPIER_HEADER_SIZE, bytes.end());
  const auto blocks = rom.size() / INTERLEAVE_BLOCK_SIZE;

  for (size_t block = 0; block < blocks; ++block) {
    const auto start = block * INTERLEAVE_BLOCK_SIZE;
    const std::vector<uint8_t> scattered(rom.begin() + start, rom.begin() + start + INTERLEAVE_BLOCK_SIZE);
    const auto half = INTERLEAVE_BLOCK_SIZE / 2;

    for (size_t i = 0; i < half; ++i) {
      rom[start + i * 2] = scattered[half + i];
      rom[start + i * 2 + 1] = scattered[i];
    }
  }

  return rom;
}

// An mdx holds the cartridge from its fifth byte with every byte flipped, so the marker a plain
// dump carries at 0x100 sits at 0x104
bool isEncodedImage(const std::vector<uint8_t> &bytes) {
  constexpr size_t offset = SEGA_MAGIC_OFFSET + 4;
  constexpr uint8_t key = 0x40;

  return bytes.size() > offset + 4 && (bytes[offset] ^ key) == 'S' && (bytes[offset + 1] ^ key) == 'E' &&
         (bytes[offset + 2] ^ key) == 'G' && (bytes[offset + 3] ^ key) == 'A';
}

std::vector<uint8_t> decodeEncodedImage(const std::vector<uint8_t> &bytes) {
  std::vector<uint8_t> rom;
  rom.reserve(bytes.size() - 5);

  for (size_t i = 4; i + 1 < bytes.size(); ++i) {
    rom.push_back(bytes[i] ^ 0x40);
  }

  return rom;
}

// The plain cartridge inside whichever container arrived, or the bytes untouched when they already
// are one
std::vector<uint8_t> megaDriveCartridge(const std::vector<uint8_t> &bytes) {
  if (isEncodedImage(bytes)) {
    return decodeEncodedImage(bytes);
  }

  if (isCopierImage(bytes)) {
    return deinterleaveCopierImage(bytes);
  }

  return bytes;
}

} // namespace

std::string ContentHasher::md5(const uint8_t *data, const size_t len) {
  md5_state_t state;
  md5_init(&state);
  // md5_append takes an int length; feed it in chunks to stay correct for very
  // large inputs
  size_t offset = 0;
  while (offset < len) {
    const size_t chunk = std::min<size_t>(len - offset, 1u << 20);
    md5_append(&state, data + offset, static_cast<int>(chunk));
    offset += chunk;
  }
  md5_byte_t digest[16];
  md5_finish(&state, digest);
  return toHex(digest, 16);
}

ContentHasher::ContentHasher() { installRcHashLogging(); }

HashedContent ContentHasher::hash(const int platformId, const std::vector<uint8_t> &fileBytes) const {
  HashedContent result;

  switch (platformId) {
  case firelight::platforms::PlatformService::PLATFORM_ID_NES: {
    static const uint8_t NES_SIGNATURE[4] = {'N', 'E', 'S', 0x1A};
    result.contentBytes = fileBytes;
    if (startsWith(fileBytes, NES_SIGNATURE, 4)) {
      const size_t skip = std::min<size_t>(16, fileBytes.size());
      result.contentHash = md5(fileBytes.data() + skip, fileBytes.size() - skip);
    } else {
      result.contentHash = md5(fileBytes.data(), fileBytes.size());
    }
    return result;
  }

  case firelight::platforms::PlatformService::PLATFORM_ID_SNES:
    result.contentBytes = fileBytes;
    if (fileBytes.size() % 1024 == 512) {
      result.contentHash = md5(fileBytes.data() + 512, fileBytes.size() - 512);
    } else {
      result.contentHash = md5(fileBytes.data(), fileBytes.size());
    }
    return result;

  case firelight::platforms::PlatformService::PLATFORM_ID_N64: {
    static const uint8_t Z64_SIGNATURE[4] = {0x80, 0x37, 0x12, 0x40};
    static const uint8_t V64_SIGNATURE[4] = {0x37, 0x80, 0x40, 0x12};
    static const uint8_t N64_SIGNATURE[4] = {0x40, 0x12, 0x37, 0x80};

    if (startsWith(fileBytes, V64_SIGNATURE, 4)) {
      std::vector<uint8_t> other(fileBytes.size());
      for (size_t i = 0; i + 1 < fileBytes.size(); i += 2) {
        other[i] = fileBytes[i + 1];
        other[i + 1] = fileBytes[i];
      }
      result.contentHash = md5(other.data(), other.size());
      result.contentBytes = std::move(other);
    } else if (startsWith(fileBytes, N64_SIGNATURE, 4)) {
      std::vector<uint8_t> other(fileBytes.size());
      for (size_t i = 0; i + 3 < fileBytes.size(); i += 4) {
        other[i] = fileBytes[i + 3];
        other[i + 1] = fileBytes[i + 2];
        other[i + 2] = fileBytes[i + 1];
        other[i + 3] = fileBytes[i];
      }
      result.contentHash = md5(other.data(), other.size());
      result.contentBytes = std::move(other);
    } else if (startsWith(fileBytes, Z64_SIGNATURE, 4)) {
      result.contentBytes = fileBytes;
      result.contentHash = md5(fileBytes.data(), fileBytes.size());
    }
    return result;
  }

  // TODO
  // Mega Drive dumps circulate in containers that hold the same cartridge in a rearranged form, so
  // the plain one is taken out first and everything downstream sees a single representation
  case firelight::platforms::PlatformService::PLATFORM_ID_SEGA_GENESIS:
    result.contentBytes = megaDriveCartridge(fileBytes);
    result.contentHash = md5(result.contentBytes.data(), result.contentBytes.size());
    return result;

  case firelight::platforms::PlatformService::PLATFORM_ID_GAMEBOY:
  case firelight::platforms::PlatformService::PLATFORM_ID_GAMEBOY_COLOR:
  case firelight::platforms::PlatformService::PLATFORM_ID_GAMEBOY_ADVANCE:
  case firelight::platforms::PlatformService::PLATFORM_ID_SEGA_GAMEGEAR:
  case firelight::platforms::PlatformService::PLATFORM_ID_SEGA_MASTER_SYSTEM:
    result.contentBytes = fileBytes;
    result.contentHash = md5(fileBytes.data(), fileBytes.size());
    return result;

  default: {
    char hash[33] = {0};
    rc_hash_generate_from_buffer(hash,
                                 static_cast<uint32_t>(platforms::PlatformService::rcConsoleForPlatform(platformId)),
                                 fileBytes.data(), fileBytes.size());
    result.contentBytes = fileBytes;
    result.contentHash = std::string(hash);
    return result;
  }
  }
}

} // namespace firelight::library

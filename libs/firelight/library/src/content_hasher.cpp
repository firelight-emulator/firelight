#include <firelight/library/content_hasher.hpp>

#include <md5.h>
#include <rcheevos/rc_hash.h>

#include <platform_metadata.hpp>

#include <algorithm>
#include <cstring>

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

bool startsWith(const std::vector<uint8_t> &bytes, const uint8_t *signature,
                size_t n) {
  return bytes.size() >= n && std::memcmp(bytes.data(), signature, n) == 0;
}

// Maps a Firelight platform id to the rcheevos console id used for buffer-based
// content hashing of cartridge systems that rcheevos handles directly.
uint32_t rcConsoleForPlatform(int platformId) {
  switch (platformId) {
  case PlatformMetadata::PLATFORM_ID_GAMEBOY:
    return RC_CONSOLE_GAMEBOY;
  case PlatformMetadata::PLATFORM_ID_GAMEBOY_COLOR:
    return RC_CONSOLE_GAMEBOY_COLOR;
  case PlatformMetadata::PLATFORM_ID_GAMEBOY_ADVANCE:
    return RC_CONSOLE_GAMEBOY_ADVANCE;
  case PlatformMetadata::PLATFORM_ID_VIRTUAL_BOY:
    return RC_CONSOLE_VIRTUAL_BOY;
  case PlatformMetadata::PLATFORM_ID_NES:
    return RC_CONSOLE_NINTENDO;
  case PlatformMetadata::PLATFORM_ID_SNES:
    return RC_CONSOLE_SUPER_NINTENDO;
  case PlatformMetadata::PLATFORM_ID_N64:
    return RC_CONSOLE_NINTENDO_64;
  case PlatformMetadata::PLATFORM_ID_NINTENDO_DS:
    return RC_CONSOLE_NINTENDO_DS;
  case PlatformMetadata::PLATFORM_ID_SEGA_MASTER_SYSTEM:
    return RC_CONSOLE_MASTER_SYSTEM;
  case PlatformMetadata::PLATFORM_ID_SEGA_GENESIS:
    return RC_CONSOLE_MEGA_DRIVE;
  case PlatformMetadata::PLATFORM_ID_SEGA_GAMEGEAR:
    return RC_CONSOLE_GAME_GEAR;
  case PlatformMetadata::PLATFORM_ID_SEGA_SATURN:
    return RC_CONSOLE_SATURN;
  case PlatformMetadata::PLATFORM_ID_SEGA_32X:
    return RC_CONSOLE_SEGA_32X;
  case PlatformMetadata::PLATFORM_ID_SEGA_CD:
    return RC_CONSOLE_SEGA_CD;
  case PlatformMetadata::PLATFORM_ID_PS1:
    return RC_CONSOLE_PLAYSTATION;
  case PlatformMetadata::PLATFORM_ID_PS2:
    return RC_CONSOLE_PLAYSTATION_2;
  case PlatformMetadata::PLATFORM_ID_PLAYSTATION_PORTABLE:
    return RC_CONSOLE_PSP;
  case PlatformMetadata::PLATFORM_ID_SUPERGRAFX:
  case PlatformMetadata::PLATFORM_ID_TURBOGRAFX16:
    return RC_CONSOLE_PC_ENGINE;
  case PlatformMetadata::PLATFORM_ID_POKEMON_MINI:
    return RC_CONSOLE_POKEMON_MINI;
  case PlatformMetadata::PLATFORM_ID_WONDERSWAN:
    return RC_CONSOLE_WONDERSWAN;
  case PlatformMetadata::PLATFORM_ID_SG1000:
    return RC_CONSOLE_SG1000;
  case PlatformMetadata::PLATFORM_ID_NEOGEO_POCKET:
    return RC_CONSOLE_NEOGEO_POCKET;
  default:
    return RC_CONSOLE_UNKNOWN;
  }
}
} // namespace

std::string ContentHasher::md5(const uint8_t *data, const size_t len) {
  md5_state_t state;
  md5_init(&state);
  // md5_append takes an int length; feed it in chunks to stay correct for very
  // large inputs.
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

HashedContent ContentHasher::hash(const int platformId,
                                  const std::vector<uint8_t> &fileBytes) const {
  HashedContent result;

  switch (platformId) {
  case PlatformMetadata::PLATFORM_ID_NES: {
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

  case PlatformMetadata::PLATFORM_ID_SNES:
    result.contentBytes = fileBytes;
    if (fileBytes.size() % 1024 == 512) {
      result.contentHash =
          md5(fileBytes.data() + 512, fileBytes.size() - 512);
    } else {
      result.contentHash = md5(fileBytes.data(), fileBytes.size());
    }
    return result;

  case PlatformMetadata::PLATFORM_ID_N64: {
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

  case PlatformMetadata::PLATFORM_ID_GAMEBOY:
  case PlatformMetadata::PLATFORM_ID_GAMEBOY_COLOR:
  case PlatformMetadata::PLATFORM_ID_GAMEBOY_ADVANCE:
  case PlatformMetadata::PLATFORM_ID_SEGA_GENESIS:
  case PlatformMetadata::PLATFORM_ID_SEGA_GAMEGEAR:
  case PlatformMetadata::PLATFORM_ID_SEGA_MASTER_SYSTEM:
    result.contentBytes = fileBytes;
    result.contentHash = md5(fileBytes.data(), fileBytes.size());
    return result;

  default: {
    char hash[33] = {0};
    rc_hash_generate_from_buffer(hash, rcConsoleForPlatform(platformId),
                                 fileBytes.data(), fileBytes.size());
    result.contentBytes = fileBytes;
    result.contentHash = std::string(hash);
    return result;
  }
  }
}

} // namespace firelight::library

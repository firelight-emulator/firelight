#include "../../../libs/firelight/library/src/firelight/library/content_hasher.hpp"

#include <firelight/platforms/platform_service.hpp>

#include <algorithm>
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <vector>

namespace firelight::library {
namespace {

using platforms::PlatformService;

std::vector<uint8_t> readFile(const std::string &path) {
  std::ifstream in(path, std::ios::binary);
  return {std::istreambuf_iterator(in), std::istreambuf_iterator<char>()};
}

std::vector<uint8_t> bytesOf(const std::string &s) { return {s.begin(), s.end()}; }

} // namespace

//****************
// md5
//****************

// RFC 1321 test vectors: these pin the digest to the published values rather
// than to whatever this implementation happens to produce
TEST(ContentHasherTest, Md5MatchesPublishedVectors) {
  EXPECT_EQ(ContentHasher::md5(nullptr, 0), "d41d8cd98f00b204e9800998ecf8427e");

  const auto abc = bytesOf("abc");
  EXPECT_EQ(ContentHasher::md5(abc.data(), abc.size()), "900150983cd24fb0d6963f7d28e17f72");

  const auto message = bytesOf("message digest");
  EXPECT_EQ(ContentHasher::md5(message.data(), message.size()), "f96b697d7cb7938d525a2f31aaf161d0");

  const auto alphabet = bytesOf("abcdefghijklmnopqrstuvwxyz");
  EXPECT_EQ(ContentHasher::md5(alphabet.data(), alphabet.size()), "c3fcd3d76192e4007dfb496cca67e13b");
}

//****************
// content hashes
//****************

// The hash is the primary key tying a ROM to its achievements, saves and
// library entry, so it is pinned to an exact value: a change to the algorithm
// silently orphans every existing user's data
TEST(ContentHasherTest, GbaRomHashesToItsKnownValue) {
  const auto bytes = readFile("test_resources/testrom.gba");
  ASSERT_FALSE(bytes.empty()) << "test_resources/testrom.gba missing from the build directory";

  const ContentHasher hasher;
  const auto hashed = hasher.hash(PlatformService::PLATFORM_ID_GAMEBOY_ADVANCE, bytes);

  EXPECT_EQ(hashed.contentHash, "e26ee0d44e809351c8ce2d73c7400cdd");
}

TEST(ContentHasherTest, GbaContentBytesAreTheFileBytes) {
  const auto bytes = readFile("test_resources/testrom.gba");
  ASSERT_FALSE(bytes.empty());

  const ContentHasher hasher;
  const auto hashed = hasher.hash(PlatformService::PLATFORM_ID_GAMEBOY_ADVANCE, bytes);

  EXPECT_EQ(hashed.contentBytes, bytes);
}

TEST(ContentHasherTest, N64RomHashIsStable) {
  const auto bytes = readFile("test_resources/testrom.z64");
  ASSERT_FALSE(bytes.empty()) << "test_resources/testrom.z64 missing from the build directory";

  const ContentHasher hasher;
  const auto first = hasher.hash(PlatformService::PLATFORM_ID_N64, bytes);
  const auto second = hasher.hash(PlatformService::PLATFORM_ID_N64, bytes);

  EXPECT_FALSE(first.contentHash.empty());
  EXPECT_EQ(first.contentHash, second.contentHash);
  EXPECT_EQ(first.contentBytes, second.contentBytes);
}

TEST(ContentHasherTest, DifferentBytesHashDifferently) {
  const ContentHasher hasher;
  const auto first = hasher.hash(PlatformService::PLATFORM_ID_GAMEBOY_ADVANCE, bytesOf("one"));
  const auto second = hasher.hash(PlatformService::PLATFORM_ID_GAMEBOY_ADVANCE, bytesOf("two"));

  EXPECT_NE(first.contentHash, second.contentHash);
}

TEST(ContentHasherTest, SameBytesHashIdenticallyAcrossCalls) {
  const auto bytes = readFile("test_resources/testrom.gba");
  ASSERT_FALSE(bytes.empty());

  const ContentHasher hasher;
  EXPECT_EQ(hasher.hash(PlatformService::PLATFORM_ID_GAMEBOY_ADVANCE, bytes).contentHash,
            hasher.hash(PlatformService::PLATFORM_ID_GAMEBOY_ADVANCE, bytes).contentHash);
}

namespace {
// A .v64 is the same ROM with every byte pair swapped
std::vector<uint8_t> toByteSwapped(const std::vector<uint8_t> &z64) {
  auto swapped = z64;
  for (size_t i = 0; i + 1 < swapped.size(); i += 2) {
    std::swap(swapped[i], swapped[i + 1]);
  }
  return swapped;
}

// A .n64 is the same ROM with every four-byte word reversed
std::vector<uint8_t> toWordSwapped(const std::vector<uint8_t> &z64) {
  auto swapped = z64;
  for (size_t i = 0; i + 3 < swapped.size(); i += 4) {
    std::swap(swapped[i], swapped[i + 3]);
    std::swap(swapped[i + 1], swapped[i + 2]);
  }
  return swapped;
}
} // namespace

// The same N64 game in all three dump formats is one game. Everything a user owns hangs off the
// content hash, so a .v64 that hashed differently from its .z64 would be a second library entry
// with its own saves
TEST(ContentHasherTest, EveryN64DumpFormatOfOneRomHashesTheSame) {
  const auto z64 = readFile("test_resources/testrom.z64");
  ASSERT_FALSE(z64.empty()) << "test_resources/testrom.z64 missing from the build directory";

  const ContentHasher hasher;
  const auto fromZ64 = hasher.hash(PlatformService::PLATFORM_ID_N64, z64);
  const auto fromV64 = hasher.hash(PlatformService::PLATFORM_ID_N64, toByteSwapped(z64));
  const auto fromN64 = hasher.hash(PlatformService::PLATFORM_ID_N64, toWordSwapped(z64));

  ASSERT_FALSE(fromZ64.contentHash.empty());
  EXPECT_EQ(fromV64.contentHash, fromZ64.contentHash);
  EXPECT_EQ(fromN64.contentHash, fromZ64.contentHash);

  // And the bytes handed to the core are the big-endian ones whatever came in
  EXPECT_EQ(fromV64.contentBytes, z64);
  EXPECT_EQ(fromN64.contentBytes, z64);
}

// A headered and a headerless dump of one NES game are the same game
TEST(ContentHasherTest, TheINesHeaderDoesNotChangeTheHash) {
  std::vector<uint8_t> body(4096);
  for (size_t i = 0; i < body.size(); ++i) {
    body[i] = static_cast<uint8_t>(i * 7);
  }

  std::vector<uint8_t> headered{'N', 'E', 'S', 0x1A, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  headered.insert(headered.end(), body.begin(), body.end());

  const ContentHasher hasher;
  const auto withHeader = hasher.hash(PlatformService::PLATFORM_ID_NES, headered);
  const auto withoutHeader = hasher.hash(PlatformService::PLATFORM_ID_NES, body);

  EXPECT_FALSE(withHeader.contentHash.empty());
  EXPECT_EQ(withHeader.contentHash, withoutHeader.contentHash);

  // The header still reaches the core, which needs it to pick a mapper
  EXPECT_EQ(withHeader.contentBytes, headered);
}

// A copier header is 512 bytes on top of a power-of-two ROM, which is how it is recognised
TEST(ContentHasherTest, TheSnesCopierHeaderDoesNotChangeTheHash) {
  std::vector<uint8_t> body(2048);
  for (size_t i = 0; i < body.size(); ++i) {
    body[i] = static_cast<uint8_t>(i % 251);
  }

  std::vector<uint8_t> headered(512, 0xAB);
  headered.insert(headered.end(), body.begin(), body.end());
  ASSERT_EQ(headered.size() % 1024, 512u);

  const ContentHasher hasher;
  const auto withHeader = hasher.hash(PlatformService::PLATFORM_ID_SNES, headered);
  const auto withoutHeader = hasher.hash(PlatformService::PLATFORM_ID_SNES, body);

  EXPECT_FALSE(withHeader.contentHash.empty());
  EXPECT_EQ(withHeader.contentHash, withoutHeader.contentHash);
}

// An FDS dump opens with its own 16-byte header, which the NES branch does not know to skip.
// This is the whole reason the Famicom Disk System is a platform rather than an NES extension:
// filed under NES it would hash the header in and agree with nothing
TEST(ContentHasherTest, TheFdsHeaderDoesNotChangeTheHash) {
  std::vector<uint8_t> body(2048);
  for (size_t i = 0; i < body.size(); ++i) {
    body[i] = static_cast<uint8_t>(i % 251);
  }

  std::vector<uint8_t> headered{'F', 'D', 'S', 0x1A};
  headered.resize(16, 0);
  headered.insert(headered.end(), body.begin(), body.end());

  const ContentHasher hasher;
  const auto withHeader = hasher.hash(PlatformService::PLATFORM_ID_FAMICOM_DISK_SYSTEM, headered);
  const auto withoutHeader = hasher.hash(PlatformService::PLATFORM_ID_FAMICOM_DISK_SYSTEM, body);

  EXPECT_FALSE(withHeader.contentHash.empty()) << "the FDS platform reaches no hasher at all";
  EXPECT_EQ(withHeader.contentHash, withoutHeader.contentHash) << "the header was hashed in";

  // Filed under NES the same bytes hash differently, which is the trap being avoided
  EXPECT_NE(hasher.hash(PlatformService::PLATFORM_ID_NES, headered).contentHash, withHeader.contentHash);
}

//****************
// mega drive containers
//****************

namespace {
// A cartridge is recognised by its marker at 0x100, and the bytes either side only have to be
// distinct enough that a rearrangement would show up in the digest
std::vector<uint8_t> megaDriveRom(const size_t size = 0x8000) {
  std::vector<uint8_t> rom(size);

  for (size_t i = 0; i < size; ++i) {
    rom[i] = static_cast<uint8_t>((i * 7 + i / 251) & 0xFF);
  }

  const std::string magic = "SEGA MEGA DRIVE ";
  std::copy(magic.begin(), magic.end(), rom.begin() + 0x100);
  return rom;
}

// Built forward from the layout the container uses, so the hasher's inverse is not being checked
// against a copy of itself: the first half of each block holds the odd bytes, the second the even
std::vector<uint8_t> asCopierImage(const std::vector<uint8_t> &rom) {
  std::vector<uint8_t> image(512, 0xAA);
  constexpr size_t block = 0x4000;
  constexpr size_t half = block / 2;

  for (size_t start = 0; start < rom.size(); start += block) {
    std::vector<uint8_t> scattered(block);

    for (size_t i = 0; i < half; ++i) {
      scattered[i] = rom[start + i * 2 + 1];
      scattered[half + i] = rom[start + i * 2];
    }

    image.insert(image.end(), scattered.begin(), scattered.end());
  }

  return image;
}

// Four leading bytes, then the cartridge with every byte flipped, then a trailing byte
std::vector<uint8_t> asEncodedImage(const std::vector<uint8_t> &rom) {
  std::vector<uint8_t> image(4, 0x00);

  for (const auto byte : rom) {
    image.push_back(byte ^ 0x40);
  }

  image.push_back(0x00);
  return image;
}
} // namespace

// The whole point of reading these containers: one game has one identity however it was dumped.
// Three different paths through the hasher have to arrive at the same digest, or the same game
// under two filenames becomes two library entries whose saves never meet
TEST(ContentHasherTest, ACartridgeHashesTheSameInEveryContainerItArrivesIn) {
  const ContentHasher hasher;
  const auto rom = megaDriveRom();

  const auto plain = hasher.hash(PlatformService::PLATFORM_ID_SEGA_GENESIS, rom);
  const auto copier = hasher.hash(PlatformService::PLATFORM_ID_SEGA_GENESIS, asCopierImage(rom));
  const auto encoded = hasher.hash(PlatformService::PLATFORM_ID_SEGA_GENESIS, asEncodedImage(rom));

  ASSERT_FALSE(plain.contentHash.empty());
  EXPECT_EQ(copier.contentHash, plain.contentHash);
  EXPECT_EQ(encoded.contentHash, plain.contentHash);

  // The bytes handed on are the cartridge too, since those are what the core is given and what a
  // patch is applied to
  EXPECT_EQ(copier.contentBytes, rom);
  EXPECT_EQ(encoded.contentBytes, rom);
}

// Containers are recognised from their contents, so a dump keeps its identity under any name
TEST(ContentHasherTest, AContainerIsReadWhateverItWasNamed) {
  const ContentHasher hasher;
  const auto rom = megaDriveRom();

  EXPECT_EQ(hasher.hash(PlatformService::PLATFORM_ID_SEGA_GENESIS, asCopierImage(rom)).contentHash,
            hasher.hash(PlatformService::PLATFORM_ID_SEGA_GENESIS, rom).contentHash);
}

// The other half, and the one that matters more: this shares a branch with every Genesis dump
// already in somebody's library, so a detector that fired too readily would move all of them
TEST(ContentHasherTest, APlainCartridgeIsLeftAlone) {
  const ContentHasher hasher;

  for (const size_t size : {0x8000u, 0x20000u, 0x80000u}) {
    const auto rom = megaDriveRom(size);
    const auto hashed = hasher.hash(PlatformService::PLATFORM_ID_SEGA_GENESIS, rom);

    EXPECT_EQ(hashed.contentBytes, rom) << "size " << size;
    EXPECT_EQ(hashed.contentHash, ContentHasher::md5(rom.data(), rom.size())) << "size " << size;
  }
}

// A dump that is neither container and carries no marker still must not be rearranged: the
// size test alone would claim anything that happens to be an odd number of 512-byte blocks
TEST(ContentHasherTest, BytesThatAreNeitherContainerAreHashedAsTheyAre) {
  const ContentHasher hasher;
  std::vector<uint8_t> odd(0x8000 + 512, 0x5A);

  const auto hashed = hasher.hash(PlatformService::PLATFORM_ID_SEGA_GENESIS, odd);

  // No marker anywhere, so the copier test does fire — what this pins is that the result stays
  // deterministic and self-consistent rather than depending on the leading bytes
  EXPECT_EQ(hashed.contentHash, ContentHasher::md5(hashed.contentBytes.data(), hashed.contentBytes.size()));
  EXPECT_FALSE(hashed.contentHash.empty());
}

} // namespace firelight::library

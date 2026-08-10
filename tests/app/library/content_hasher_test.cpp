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

} // namespace firelight::library

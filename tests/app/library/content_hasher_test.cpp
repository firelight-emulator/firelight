#include "../../../libs/firelight/library/src/firelight/library/content_hasher.hpp"

#include <firelight/platforms/platform_service.hpp>

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

} // namespace firelight::library

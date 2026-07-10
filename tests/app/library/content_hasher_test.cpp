#include <firelight/library/content_hasher.hpp>

#include <firelight/platforms/platform_service.hpp>

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

// These tests pin down the invariant the mod shop relies on: the canonical
// content hash is *source-independent*. A standalone pre-patched ROM and a
// base ROM that has been hot-patched produce the same game bytes, so — after
// header/byte-order normalization — they hash identically and therefore share
// the same content-addressed save directory. If this ever regressed, a mod's
// two install paths would silently stop sharing progress.
namespace firelight::library {
namespace {
using platforms::PlatformService;

std::vector<uint8_t> pattern(size_t n, uint8_t seed) {
  std::vector<uint8_t> out(n);
  for (size_t i = 0; i < n; ++i) {
    out[i] = static_cast<uint8_t>((i * 7 + seed) & 0xFF);
  }
  return out;
}

std::vector<uint8_t> concat(std::vector<uint8_t> a,
                            const std::vector<uint8_t> &b) {
  a.insert(a.end(), b.begin(), b.end());
  return a;
}
} // namespace

TEST(ContentHasherEquivalenceTest, NesHeaderedMatchesHeaderless) {
  ContentHasher hasher;
  // The game body, as a headerless pre-patched ROM would ship it. Chosen so it
  // does not itself begin with the iNES signature.
  const auto body = pattern(4096, 0x11);

  std::vector<uint8_t> header = {'N', 'E', 'S', 0x1A, 1, 1, 0, 0,
                                 0,   0,   0,   0,    0, 0, 0, 0};
  const auto headered = concat(header, body); // base ROM + patch output form

  EXPECT_EQ(hasher.hash(PlatformService::PLATFORM_ID_NES, headered).contentHash,
            hasher.hash(PlatformService::PLATFORM_ID_NES, body).contentHash);
}

TEST(ContentHasherEquivalenceTest, SnesCopierHeaderedMatchesHeaderless) {
  ContentHasher hasher;
  // Body size is a multiple of 1024, so the headerless form is not mistaken for
  // a copier-headered one, and the +512 headered form is.
  const auto body = pattern(2048, 0x22);
  const auto header = pattern(512, 0x99);
  const auto headered = concat(header, body);

  ASSERT_EQ(headered.size() % 1024, 512u);
  EXPECT_EQ(hasher.hash(PlatformService::PLATFORM_ID_SNES, headered).contentHash,
            hasher.hash(PlatformService::PLATFORM_ID_SNES, body).contentHash);
}

TEST(ContentHasherEquivalenceTest, N64ByteOrderVariantsMatch) {
  ContentHasher hasher;
  // Big-endian (.z64) canonical bytes.
  std::vector<uint8_t> z64 = {0x80, 0x37, 0x12, 0x40};
  const auto tail = pattern(4092, 0x33); // keep total a multiple of 4
  z64 = concat(z64, tail);

  // Byte-swapped little-endian (.v64) form of the same ROM.
  std::vector<uint8_t> v64(z64.size());
  for (size_t i = 0; i + 1 < z64.size(); i += 2) {
    v64[i] = z64[i + 1];
    v64[i + 1] = z64[i];
  }
  ASSERT_EQ(v64[0], 0x37);
  ASSERT_EQ(v64[1], 0x80);

  EXPECT_EQ(hasher.hash(PlatformService::PLATFORM_ID_N64, v64).contentHash,
            hasher.hash(PlatformService::PLATFORM_ID_N64, z64).contentHash);
}

TEST(ContentHasherEquivalenceTest, DifferentContentHashesDiffer) {
  ContentHasher hasher;
  const auto a = pattern(1024, 0x01);
  const auto b = pattern(1024, 0x02);
  EXPECT_NE(hasher.hash(PlatformService::PLATFORM_ID_GAMEBOY, a).contentHash,
            hasher.hash(PlatformService::PLATFORM_ID_GAMEBOY, b).contentHash);
}

} // namespace firelight::library

#pragma once

// Compact, dependency-free MD5 (replaces QCryptographicHash for the savefile
// dedup fingerprint). MD5 is used only to detect unchanged save bytes and skip
// a redundant write — not for security. Public-domain style implementation

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

namespace firelight::saves::detail {

class Md5 {
public:
  Md5() { reset(); }

  void update(const void *data, size_t len) {
    const auto *p = static_cast<const uint8_t *>(data);
    size_t index = static_cast<size_t>((m_count >> 3) & 0x3f);
    m_count += static_cast<uint64_t>(len) << 3;
    const size_t partLen = 64 - index;
    size_t i = 0;
    if (len >= partLen) {
      for (size_t j = 0; j < partLen; ++j) {
        m_buffer[index + j] = p[j];
      }
      transform(m_buffer.data());
      for (i = partLen; i + 63 < len; i += 64) {
        transform(p + i);
      }
      index = 0;
    }
    for (size_t j = 0; i + j < len; ++j) {
      m_buffer[index + j] = p[i + j];
    }
  }

  std::string hexDigest() {
    static const uint8_t padding[64] = {0x80};
    uint8_t bits[8];
    encode(bits, &m_count, 8);

    const size_t index = static_cast<size_t>((m_count >> 3) & 0x3f);
    const size_t padLen = (index < 56) ? (56 - index) : (120 - index);
    update(padding, padLen);
    update(bits, 8);

    uint8_t digest[16];
    encode(digest, m_state.data(), 16);

    static const char *hex = "0123456789abcdef";
    std::string out;
    out.reserve(32);
    for (uint8_t b : digest) {
      out.push_back(hex[b >> 4]);
      out.push_back(hex[b & 0x0f]);
    }
    return out;
  }

  static std::string hash(const void *data, size_t len) {
    Md5 md5;
    md5.update(data, len);
    return md5.hexDigest();
  }

private:
  void reset() {
    m_count = 0;
    m_state = {0x67452301u, 0xefcdab89u, 0x98badcfeu, 0x10325476u};
  }

  static uint32_t rotl(uint32_t x, int c) { return (x << c) | (x >> (32 - c)); }

  static void encode(uint8_t *out, const void *in, size_t len) {
    const auto *p = static_cast<const uint32_t *>(in);
    for (size_t i = 0, j = 0; j < len; ++i, j += 4) {
      out[j] = static_cast<uint8_t>(p[i] & 0xff);
      out[j + 1] = static_cast<uint8_t>((p[i] >> 8) & 0xff);
      out[j + 2] = static_cast<uint8_t>((p[i] >> 16) & 0xff);
      out[j + 3] = static_cast<uint8_t>((p[i] >> 24) & 0xff);
    }
  }

  void transform(const uint8_t block[64]) {
    uint32_t a = m_state[0], b = m_state[1], c = m_state[2], d = m_state[3];
    uint32_t x[16];
    for (int i = 0, j = 0; i < 16; ++i, j += 4) {
      x[i] = static_cast<uint32_t>(block[j]) | (static_cast<uint32_t>(block[j + 1]) << 8) |
             (static_cast<uint32_t>(block[j + 2]) << 16) | (static_cast<uint32_t>(block[j + 3]) << 24);
    }

    auto ff = [](uint32_t &a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, int s, uint32_t t) {
      a = b + rotl(a + ((b & c) | (~b & d)) + x + t, s);
    };
    auto gg = [](uint32_t &a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, int s, uint32_t t) {
      a = b + rotl(a + ((b & d) | (c & ~d)) + x + t, s);
    };
    auto hh = [](uint32_t &a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, int s, uint32_t t) {
      a = b + rotl(a + (b ^ c ^ d) + x + t, s);
    };
    auto ii = [](uint32_t &a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, int s, uint32_t t) {
      a = b + rotl(a + (c ^ (b | ~d)) + x + t, s);
    };

    ff(a, b, c, d, x[0], 7, 0xd76aa478);
    ff(d, a, b, c, x[1], 12, 0xe8c7b756);
    ff(c, d, a, b, x[2], 17, 0x242070db);
    ff(b, c, d, a, x[3], 22, 0xc1bdceee);
    ff(a, b, c, d, x[4], 7, 0xf57c0faf);
    ff(d, a, b, c, x[5], 12, 0x4787c62a);
    ff(c, d, a, b, x[6], 17, 0xa8304613);
    ff(b, c, d, a, x[7], 22, 0xfd469501);
    ff(a, b, c, d, x[8], 7, 0x698098d8);
    ff(d, a, b, c, x[9], 12, 0x8b44f7af);
    ff(c, d, a, b, x[10], 17, 0xffff5bb1);
    ff(b, c, d, a, x[11], 22, 0x895cd7be);
    ff(a, b, c, d, x[12], 7, 0x6b901122);
    ff(d, a, b, c, x[13], 12, 0xfd987193);
    ff(c, d, a, b, x[14], 17, 0xa679438e);
    ff(b, c, d, a, x[15], 22, 0x49b40821);

    gg(a, b, c, d, x[1], 5, 0xf61e2562);
    gg(d, a, b, c, x[6], 9, 0xc040b340);
    gg(c, d, a, b, x[11], 14, 0x265e5a51);
    gg(b, c, d, a, x[0], 20, 0xe9b6c7aa);
    gg(a, b, c, d, x[5], 5, 0xd62f105d);
    gg(d, a, b, c, x[10], 9, 0x02441453);
    gg(c, d, a, b, x[15], 14, 0xd8a1e681);
    gg(b, c, d, a, x[4], 20, 0xe7d3fbc8);
    gg(a, b, c, d, x[9], 5, 0x21e1cde6);
    gg(d, a, b, c, x[14], 9, 0xc33707d6);
    gg(c, d, a, b, x[3], 14, 0xf4d50d87);
    gg(b, c, d, a, x[8], 20, 0x455a14ed);
    gg(a, b, c, d, x[13], 5, 0xa9e3e905);
    gg(d, a, b, c, x[2], 9, 0xfcefa3f8);
    gg(c, d, a, b, x[7], 14, 0x676f02d9);
    gg(b, c, d, a, x[12], 20, 0x8d2a4c8a);

    hh(a, b, c, d, x[5], 4, 0xfffa3942);
    hh(d, a, b, c, x[8], 11, 0x8771f681);
    hh(c, d, a, b, x[11], 16, 0x6d9d6122);
    hh(b, c, d, a, x[14], 23, 0xfde5380c);
    hh(a, b, c, d, x[1], 4, 0xa4beea44);
    hh(d, a, b, c, x[4], 11, 0x4bdecfa9);
    hh(c, d, a, b, x[7], 16, 0xf6bb4b60);
    hh(b, c, d, a, x[10], 23, 0xbebfbc70);
    hh(a, b, c, d, x[13], 4, 0x289b7ec6);
    hh(d, a, b, c, x[0], 11, 0xeaa127fa);
    hh(c, d, a, b, x[3], 16, 0xd4ef3085);
    hh(b, c, d, a, x[6], 23, 0x04881d05);
    hh(a, b, c, d, x[9], 4, 0xd9d4d039);
    hh(d, a, b, c, x[12], 11, 0xe6db99e5);
    hh(c, d, a, b, x[15], 16, 0x1fa27cf8);
    hh(b, c, d, a, x[2], 23, 0xc4ac5665);

    ii(a, b, c, d, x[0], 6, 0xf4292244);
    ii(d, a, b, c, x[7], 10, 0x432aff97);
    ii(c, d, a, b, x[14], 15, 0xab9423a7);
    ii(b, c, d, a, x[5], 21, 0xfc93a039);
    ii(a, b, c, d, x[12], 6, 0x655b59c3);
    ii(d, a, b, c, x[3], 10, 0x8f0ccc92);
    ii(c, d, a, b, x[10], 15, 0xffeff47d);
    ii(b, c, d, a, x[1], 21, 0x85845dd1);
    ii(a, b, c, d, x[8], 6, 0x6fa87e4f);
    ii(d, a, b, c, x[15], 10, 0xfe2ce6e0);
    ii(c, d, a, b, x[6], 15, 0xa3014314);
    ii(b, c, d, a, x[13], 21, 0x4e0811a1);
    ii(a, b, c, d, x[4], 6, 0xf7537e82);
    ii(d, a, b, c, x[11], 10, 0xbd3af235);
    ii(c, d, a, b, x[2], 15, 0x2ad7d2bb);
    ii(b, c, d, a, x[9], 21, 0xeb86d391);

    m_state[0] += a;
    m_state[1] += b;
    m_state[2] += c;
    m_state[3] += d;
  }

  std::array<uint32_t, 4> m_state{};
  uint64_t m_count = 0; // bit count
  std::array<uint8_t, 64> m_buffer{};
};

} // namespace firelight::saves::detail

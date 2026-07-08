#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace firelight::netplay {

inline std::string base64Encode(std::span<const uint8_t> data) {
  static constexpr char ALPHABET[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  std::string out;
  out.reserve((data.size() + 2) / 3 * 4);
  size_t i = 0;
  while (i + 2 < data.size()) {
    const uint32_t n = (data[i] << 16) | (data[i + 1] << 8) | data[i + 2];
    out += ALPHABET[(n >> 18) & 63];
    out += ALPHABET[(n >> 12) & 63];
    out += ALPHABET[(n >> 6) & 63];
    out += ALPHABET[n & 63];
    i += 3;
  }
  if (i + 1 == data.size()) {
    const uint32_t n = data[i] << 16;
    out += ALPHABET[(n >> 18) & 63];
    out += ALPHABET[(n >> 12) & 63];
    out += "==";
  } else if (i + 2 == data.size()) {
    const uint32_t n = (data[i] << 16) | (data[i + 1] << 8);
    out += ALPHABET[(n >> 18) & 63];
    out += ALPHABET[(n >> 12) & 63];
    out += ALPHABET[(n >> 6) & 63];
    out += '=';
  }
  return out;
}

inline std::optional<std::vector<uint8_t>> base64Decode(const std::string &text) {
  const auto value = [](const char c) -> int {
    if (c >= 'A' && c <= 'Z') {
      return c - 'A';
    }
    if (c >= 'a' && c <= 'z') {
      return c - 'a' + 26;
    }
    if (c >= '0' && c <= '9') {
      return c - '0' + 52;
    }
    if (c == '+') {
      return 62;
    }
    if (c == '/') {
      return 63;
    }
    return -1;
  };

  std::vector<uint8_t> out;
  out.reserve(text.size() / 4 * 3);
  uint32_t buffer = 0;
  int bits = 0;
  for (const char c : text) {
    if (c == '=') {
      break;
    }
    const int v = value(c);
    if (v < 0) {
      return std::nullopt;
    }
    buffer = (buffer << 6) | static_cast<uint32_t>(v);
    bits += 6;
    if (bits >= 8) {
      bits -= 8;
      out.push_back(static_cast<uint8_t>((buffer >> bits) & 0xFF));
    }
  }
  return out;
}

inline std::string base64EncodeText(const std::string &text) {
  return base64Encode(std::span(
      reinterpret_cast<const uint8_t *>(text.data()), text.size()));
}

inline std::optional<std::string> base64DecodeText(const std::string &text) {
  const auto bytes = base64Decode(text);
  if (!bytes) {
    return std::nullopt;
  }
  return std::string(bytes->begin(), bytes->end());
}

} // namespace firelight::netplay

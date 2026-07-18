#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace firelight::library {

// Reads an entire file into memory with a single sized read. This is markedly
// faster than the std::istreambuf_iterator idiom for large files (e.g. DS ROMs,
// which run to hundreds of MB), which reads a byte at a time and repeatedly
// reallocates. Returns an empty vector if the file can't be sized or opened
inline std::vector<uint8_t> readAllBytes(const std::string &path) {
  std::error_code ec;
  const auto size = std::filesystem::file_size(path, ec);
  if (ec) {
    return {};
  }

  std::ifstream file(path, std::ios::binary);
  if (!file) {
    return {};
  }

  std::vector<uint8_t> bytes(static_cast<size_t>(size));
  if (size > 0) {
    file.read(reinterpret_cast<char *>(bytes.data()),
              static_cast<std::streamsize>(size));
    bytes.resize(static_cast<size_t>(file.gcount()));
  }
  return bytes;
}

} // namespace firelight::library

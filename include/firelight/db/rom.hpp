#pragma once

#include <string>

namespace firelight::db {
struct ROM {
  int id = -1;
  std::string filename;
  int size_bytes;
  int gameId = -1;
  bool gameIdVerified = false;
  int platformId = -1;
  std::string md5;
  std::string sha1;
  std::string crc32;
};
} // namespace firelight::db

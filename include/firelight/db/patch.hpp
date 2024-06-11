#pragma once

#include <string>

namespace firelight::db {
struct Patch {
  int id = -1;
  std::string version;
  std::string filename;
  int sizeBytes;
  int modId = -1;
  int romId = -1;
  std::string md5;
  std::string sha1;
  std::string crc32;
};
} // namespace firelight::db
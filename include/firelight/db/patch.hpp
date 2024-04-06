#pragma once
#include <string>
#include <vector>

namespace firelight::db {
struct Patch {
  int id = -1;
  std::string filename;
  int sizeBytes;
  int modReleaseId = -1;
  std::string md5;
  std::string crc32;
  std::string sha1; // TODO:???
  std::vector<int> romIds;
};
} // namespace firelight::db
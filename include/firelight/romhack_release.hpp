#pragma once

#include <string>

struct RomhackRelease {
  int id{};
  std::string filename{};
  std::string file_ext{};
  int romhack{};
  std::string version{};
  std::string md5{};
  int size_bytes{};
};

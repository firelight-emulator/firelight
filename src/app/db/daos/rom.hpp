#pragma once

#include <string>

struct ROM {
  int id{};
  std::string filename{};
  std::string file_ext{};
  int game{};
  std::string platform{};
  std::string region{};
  std::string md5{};
  int size_bytes{};
};

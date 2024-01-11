//
// Created by alexs on 1/9/2024.
//

#ifndef ROMHACK_RELEASE_HPP
#define ROMHACK_RELEASE_HPP
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

#endif // ROMHACK_RELEASE_HPP

//
// Created by alexs on 12/22/2023.
//

#ifndef ROM_HPP
#define ROM_HPP
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

#endif // ROM_HPP

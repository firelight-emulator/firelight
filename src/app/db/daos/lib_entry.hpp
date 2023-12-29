//
// Created by alexs on 12/29/2023.
//

#ifndef LIB_ENTRY_HPP
#define LIB_ENTRY_HPP

#include <string>

struct LibEntry {
  int id{};
  std::string display_name;
  bool verified;
  std::string platform;
  int platform_id;
  std::string md5;
  int game = -1;
  int rom = -1;
  int romhack = -1;
  std::string content_path;
};

#endif // LIB_ENTRY_HPP

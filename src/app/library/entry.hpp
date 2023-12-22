//
// Created by alexs on 11/14/2023.
//

#ifndef FIRELIGHT_ENTRY_HPP
#define FIRELIGHT_ENTRY_HPP

#include <filesystem>
#include <string>
namespace FL::Library {

enum EntryType { NORMAL_GAME, ROMHACK };

struct Entry {
  int id;
  std::string display_name;
  bool verified;
  std::string platform;
  std::string md5;
  int game = -1;
  int rom = -1;
  int romhack = -1;
  std::string content_path;
};

} // namespace FL::Library

#endif // FIRELIGHT_ENTRY_HPP

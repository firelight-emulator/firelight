#pragma once

#include <string>

enum class EntryType { ROM, PATCH };

struct LibEntry {
  int id = -1;
  std::string display_name;
  EntryType type;
  bool verified;
  std::string md5;
  int platform;
  int game = -1;
  int rom = -1;
  int parent_entry = -1;
  int romhack = -1;
  int romhack_release = -1;
  std::string source_directory;
  std::string content_path;
};

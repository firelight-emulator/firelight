//
// Created by alexs on 12/29/2023.
//

#ifndef LIBRARY_DATABASE_HPP
#define LIBRARY_DATABASE_HPP

#include "daos/lib_entry.hpp"
#include <optional>
#include <vector>

class LibraryDatabase {
public:
  struct Filter {
    int id = -1;
    std::string display_name;
    bool verified;
    std::string md5;
    int platform;
    int game = -1;
    int rom = -1;
    int parent_entry = -1;
    int romhack = -1;
    std::string source_directory;
    std::string content_path;
  };

  virtual ~LibraryDatabase() = default;
  virtual void addOrRenameEntry(LibEntry entry) = 0;
  virtual std::vector<LibEntry> getAllEntries() = 0;
  virtual void match_md5s(std::string t_sourceDirectory,
                          std::vector<std::string> t_md5List) = 0;
  virtual std::vector<LibEntry> getMatching(Filter filter) = 0;
};

#endif // LIBRARY_DATABASE_HPP

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
  virtual ~LibraryDatabase() = default;
  virtual std::optional<LibEntry> get_entry_by_id(int id) = 0;
  virtual std::optional<LibEntry> get_entry_by_md5(std::string md5) = 0;
  virtual std::optional<LibEntry> get_entry_by_rom_id(int id) = 0;
  virtual void add_or_update_entry(LibEntry entry) = 0;
  virtual std::vector<LibEntry> get_all_entries() = 0;
};

#endif // LIBRARY_DATABASE_HPP

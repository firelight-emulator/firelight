//
// Created by alexs on 12/29/2023.
//

#ifndef LIBRARY_DATABASE_HPP
#define LIBRARY_DATABASE_HPP

#include "daos/lib_entry.hpp"

class LibraryDatabase {
public:
  virtual ~LibraryDatabase() = default;
  virtual LibEntry get_entry_by_id(int id) = 0;
  virtual LibEntry get_entry_by_rom_id(int id) = 0;
};

#endif // LIBRARY_DATABASE_HPP

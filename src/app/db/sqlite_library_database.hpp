//
// Created by alexs on 12/29/2023.
//

#ifndef SQLITE_LIBRARY_DATABASE_HPP
#define SQLITE_LIBRARY_DATABASE_HPP
#include "library_database.hpp"

#include <filesystem>
#include <optional>
#include <sqlite3.h>

class SqliteLibraryDatabase : public LibraryDatabase {
public:
  explicit SqliteLibraryDatabase(std::filesystem::path db_file_path);
  bool initialize();
  std::optional<LibEntry> get_entry_by_id(int id) override;
  std::optional<LibEntry> get_entry_by_md5(std::string md5) override;
  std::optional<LibEntry> get_entry_by_rom_id(int id) override;

private:
  std::filesystem::path database_file_path_;
  sqlite3 *database_ = nullptr;
};

#endif // SQLITE_LIBRARY_DATABASE_HPP

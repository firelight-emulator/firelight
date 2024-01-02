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
  ~SqliteLibraryDatabase() override = default;

  bool initialize();
  std::optional<LibEntry> get_entry_by_id(int id) override;
  std::optional<LibEntry> get_entry_by_md5(std::string md5) override;
  std::optional<LibEntry> get_entry_by_rom_id(int id) override;
  void add_or_update_entry(LibEntry entry) override;
  std::vector<LibEntry> get_all_entries() override;
  void match_md5s(std::string source_directory,
                  std::vector<std::string> md5s) override;

private:
  void insert_entry_into_db(LibEntry entry) const;
  LibEntry entry_from_stmt(sqlite3_stmt *stmt);
  std::filesystem::path database_file_path_;
  sqlite3 *database_ = nullptr;
};

#endif // SQLITE_LIBRARY_DATABASE_HPP

//
// Created by alexs on 12/29/2023.
//

#include "sqlite_library_database.hpp"

#include <utility>

const char *create_query =
    "CREATE TABLE IF NOT EXISTS library("
    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "display_name NVARCHAR(999) NOT NULL,"
    "verified INTEGER,"
    "platform NVARCHAR(999) NOT NULL,"
    "md5 NVARCHAR(999) NOT NULL UNIQUE,"
    "game INTEGER,"
    "rom INTEGER,"
    "romhack INTEGER,"
    "content_path NVARCHAR(999) NOT NULL);"
    "CREATE UNIQUE INDEX IF NOT EXISTS idx_md5 ON library (md5);";

SqliteLibraryDatabase::SqliteLibraryDatabase(std::filesystem::path db_file_path)
    : database_file_path_(std::move(db_file_path)) {}

bool SqliteLibraryDatabase::initialize() {
  // read from library file, validate entries, add to list
  // addWatchedRomDirectory(defaultRomPath);

  if (sqlite3_open(database_file_path_.string().c_str(), &database_) !=
      SQLITE_OK) {
    // std::cerr << "Cannot open database: " << sqlite3_errmsg(database)
    //           << std::endl;
    return false;
  }

  // std::cout << "Database opened successfully!" << std::endl;

  if (sqlite3_exec(database_, create_query, nullptr, nullptr, nullptr) !=
      SQLITE_OK) {
    // std::cerr << "SQL error: " << sqlite3_errmsg(database) << std::endl;
    return false;
  }
  // std::cout << "Table created successfully!" << std::endl;

  // Query data from the table
  const char *selectSQL = "SELECT * FROM library;";

  //**********************************************
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(database_, selectSQL, -1, &stmt, nullptr) !=
      SQLITE_OK) {
    // std::cerr << "SQL error: " << sqlite3_errmsg(database) << std::endl;
    sqlite3_finalize(stmt);
    return false;
  }

  sqlite3_finalize(stmt);
  return true;
}

LibEntry SqliteLibraryDatabase::get_entry_by_id(int id) {}
LibEntry SqliteLibraryDatabase::get_entry_by_rom_id(int id) {}
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

std::optional<LibEntry> SqliteLibraryDatabase::get_entry_by_id(int id) {
  return {};
}
std::optional<LibEntry>
SqliteLibraryDatabase::get_entry_by_md5(std::string md5) {
  return {};
}
std::optional<LibEntry> SqliteLibraryDatabase::get_entry_by_rom_id(int id) {
  return {};
}

void SqliteLibraryDatabase::add_or_update_entry(const LibEntry entry) {
  insert_entry_into_db(entry);
}

std::vector<LibEntry> SqliteLibraryDatabase::get_all_entries() {
  std::vector<LibEntry> results;
  sqlite3_stmt *stmt = nullptr;
  const auto query = "SELECT * FROM library;";

  if (sqlite3_prepare_v2(database_, query, -1, &stmt, nullptr) != SQLITE_OK) {
    printf("prepare didn't work: %s\n", sqlite3_errmsg(database_));
    // Handle error (use sqlite3_errmsg(db) to get the error message)
    sqlite3_finalize(
        stmt); // Always finalize a prepared statement to avoid resource leaks
    return results;
  }

  while (sqlite3_step(stmt) != SQLITE_DONE) {
    LibEntry entry;
    entry.id = sqlite3_column_int(stmt, 0);
    entry.display_name = std::string(reinterpret_cast<char *>(
        const_cast<unsigned char *>(sqlite3_column_text(stmt, 1))));
    entry.verified = sqlite3_column_int(stmt, 2);
    entry.platform_id = sqlite3_column_int(stmt, 3);
    entry.md5 = std::string(reinterpret_cast<char *>(
        const_cast<unsigned char *>(sqlite3_column_text(stmt, 4))));
    entry.game = sqlite3_column_int(stmt, 5);
    entry.rom = sqlite3_column_int(stmt, 6);
    entry.romhack = sqlite3_column_int(stmt, 7);
    entry.content_path = std::string(reinterpret_cast<char *>(
        const_cast<unsigned char *>(sqlite3_column_text(stmt, 8))));

    printf("added entry: %s\n", entry.display_name.c_str());
    results.emplace_back(entry);
    // TODO: Handle error
  }

  return results;
}

void SqliteLibraryDatabase::insert_entry_into_db(LibEntry entry) const {
  // const auto db = getOrCreateDbConnection();
  sqlite3_stmt *stmt = nullptr;
  const char *query = "INSERT OR IGNORE INTO library ("
                      "display_name, "
                      "verified, "
                      "platform,"
                      "md5, "
                      "game, "
                      "rom, "
                      "romhack, "
                      "content_path) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";

  if (sqlite3_prepare_v2(database_, query, -1, &stmt, nullptr) != SQLITE_OK) {
    printf("prepare didn't work: %s\n", sqlite3_errmsg(database_));
    // Handle error (use sqlite3_errmsg(db) to get the error message)
    sqlite3_finalize(
        stmt); // Always finalize a prepared statement to avoid resource leaks
    return;
  }

  sqlite3_bind_text(stmt, 1, entry.display_name.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_int(stmt, 2, entry.verified ? 1 : 0);
  sqlite3_bind_text(stmt, 3, entry.platform.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 4, entry.md5.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_int(stmt, 5, entry.game);
  sqlite3_bind_int(stmt, 6, entry.rom);
  sqlite3_bind_int(stmt, 7, entry.romhack);
  sqlite3_bind_text(stmt, 8, entry.content_path.c_str(), -1, SQLITE_STATIC);

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    printf("insert didn't work: %s\n", sqlite3_errmsg(database_));
    // Handle error
  } else {
    if (sqlite3_changes(database_) == 0) {
      // No new row was inserted, need to fetch the existing row's ID
      sqlite3_stmt *selectStmt = nullptr;
      const char *selectQuery = "SELECT id FROM library WHERE md5 = ?";

      if (sqlite3_prepare_v2(database_, selectQuery, -1, &selectStmt,
                             nullptr) == SQLITE_OK) {
        sqlite3_bind_text(selectStmt, 1, entry.md5.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(selectStmt) == SQLITE_ROW) {
          entry.id = sqlite3_column_int(selectStmt, 0);
        } else {
          printf("Failed to retrieve existing row ID: %s\n",
                 sqlite3_errmsg(database_));
          // Handle error
        }
        sqlite3_finalize(selectStmt);
      } else {
        printf("prepare for select didn't work: %s\n",
               sqlite3_errmsg(database_));
        // Handle error
      }
    } else {
      // A new row was inserted
      entry.id = static_cast<int>(sqlite3_last_insert_rowid(database_));
    }
  }

  // TODO: Do another query here to get the ID of the one we just put in...

  // Finalize the prepared statement to avoid resource leaks
  sqlite3_finalize(stmt);
  // entries.push_back(entry);
}
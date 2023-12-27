//
// Created by alexs on 12/13/2023.
//

#include "library_manager.hpp"
#include "../platforms/platform.hpp"
#include <fstream>
#include <iostream>
#include <openssl/evp.h>
#include <sqlite3.h>
#include <utility>
#include <vector>

namespace FL::Library {
constexpr int MAX_FILESIZE_BYTES = 50000000;
const char *createQuery =
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

static std::string calculateMD5(const char *input, int size) {
  unsigned char md5Hash[EVP_MAX_MD_SIZE];
  unsigned int md5HashLength;

  std::string output;

  // Create a message digest context
  auto mdctx = EVP_MD_CTX_new();

  // Initialize the message digest context
  if (mdctx == nullptr || !EVP_DigestInit_ex2(mdctx, EVP_md5(), nullptr)) {
    EVP_MD_CTX_free(mdctx);
    throw std::runtime_error("Failed to initialize hash context");
  }

  // Add data to the message digest context
  if (!EVP_DigestUpdate(mdctx, input, size)) {
    EVP_MD_CTX_free(mdctx);
    throw std::runtime_error("Failed to update hash context");
  }

  // Finalize the hash computation
  if (!EVP_DigestFinal_ex(mdctx, md5Hash, &md5HashLength)) {
    EVP_MD_CTX_free(mdctx);
    throw std::runtime_error("Failed to finalize hash context");
  }

  // Clean up the message digest context
  EVP_MD_CTX_free(mdctx);

  output.resize(md5HashLength * 2);
  for (unsigned int i = 0; i < md5HashLength; ++i)
    std::sprintf(&output[i * 2], "%02x", md5Hash[i]);

  return output;
}

LibraryManager::LibraryManager(std::filesystem::path libraryDb,
                               const std::filesystem::path &defaultRomPath,
                               ContentDatabase *contentDb)
    : libraryDbFile(std::move(libraryDb)), contentDatabase(contentDb) {
  // read from library file, validate entries, add to list
  addWatchedRomDirectory(defaultRomPath);

  // Open a new SQLite database (or create if not exists)
  int rc = sqlite3_open(libraryDbFile.string().c_str(), &database);

  if (rc) {
    std::cerr << "Cannot open database: " << sqlite3_errmsg(database)
              << std::endl;
    return;
  }
  std::cout << "Database opened successfully!" << std::endl;

  // Execute the SQL statement
  rc = sqlite3_exec(database, createQuery, nullptr, nullptr, nullptr);

  if (rc != SQLITE_OK) {
    std::cerr << "SQL error: " << sqlite3_errmsg(database) << std::endl;
    return;
  }
  std::cout << "Table created successfully!" << std::endl;

  // Query data from the table
  const char *selectSQL = "SELECT * FROM library;";

  //**********************************************
  sqlite3_stmt *stmt;
  rc = sqlite3_prepare_v2(database, selectSQL, -1, &stmt, nullptr);

  if (rc != SQLITE_OK) {
    std::cerr << "SQL error: " << sqlite3_errmsg(database) << std::endl;
    sqlite3_finalize(stmt);
    return;
  }

  sqlite3_finalize(stmt);

  // // Iterate through the result set
  // while (sqlite3_step(stmt) == SQLITE_ROW) {
  //   int id = sqlite3_column_int(stmt, 0);
  //   const unsigned char *displayName = sqlite3_column_text(stmt, 1);
  //   const unsigned char *md5 = sqlite3_column_text(stmt, 2);
  //   const unsigned char *filename = sqlite3_column_text(stmt, 3);
  //
  //   printf("ID: %d, display name: %s, filename: %s, md5: %s\n", id,
  //   displayName,
  //          filename, md5);
  // }
}

void LibraryManager::scanNow() {
  std::vector<FL::Library::Entry> results;

  for (const auto &path : watchedRomDirs) {
    for (const auto &entry :
         std::filesystem::recursive_directory_iterator(path)) {
      if (entry.is_directory()) {
        continue;
      }

      auto ext = entry.path().extension();
      std::string platform_display_name =
          get_display_name_by_extension(ext.string());
      if (platform_display_name.empty()) {
        continue;
      }

      auto size = entry.file_size();
      if (size > MAX_FILESIZE_BYTES) {
        continue;
      }

      std::vector<char> thing(size);
      std::ifstream file(entry.path(), std::ios::binary);

      file.read(thing.data(), size);
      file.close();

      auto md5 = calculateMD5(thing.data(), size);

      auto result = contentDatabase->getRomByMd5(md5);
      if (result.has_value()) {
        auto game = contentDatabase->getGameByRomId(result->id);

        auto displayName = result->filename;
        if (game.has_value()) {
          displayName = game->name;
        }
        // TODO: Will need to do a game lookup here as well.
        Entry e = {.id = -1,
                   .display_name = displayName,
                   .verified = true,
                   .platform = result->platform,
                   .md5 = md5,
                   .game = result->game,
                   .rom = result->id,
                   .romhack = -1,
                   .content_path = entry.path().string()};
        insertEntry(e);
      } else {
        Entry e = {.id = -1,
                   .display_name = entry.path().stem().string(),
                   .verified = false,
                   .platform =
                       platform_display_name, // TODO: Based on file extension
                   .md5 = md5,
                   .game = -1,
                   .rom = -1,
                   .romhack = -1,
                   .content_path = entry.path().string()};
        insertEntry(e);
        // TODO: Start trying other stuff
      }

      // Check for MD5 match. If a match, it's verified!
      // Otherwise, it's unverified.
      // If unverified, match the filename against the known filenames or
      // game name.
      // If THAT fails, add it to a list that the user needs to manually assign.
    }
  }
}
std::optional<Entry> LibraryManager::getEntryById(int id) {
  sqlite3_stmt *stmt = nullptr;
  auto query = "SELECT * FROM library WHERE id = ?";

  if (sqlite3_prepare_v2(database, query, -1, &stmt, nullptr) != SQLITE_OK) {
    printf("prepare didn't work\n");
    // TODO: error handle
    sqlite3_finalize(stmt);
    return {};
  }

  if (sqlite3_bind_int(stmt, 1, id) != SQLITE_OK) {
    printf("bind didn't work: %s\n", sqlite3_errmsg(database));
    // TODO: Handle error
    sqlite3_finalize(stmt);
    return {};
  }

  std::optional<Entry> result;

  if (sqlite3_step(stmt) == SQLITE_ROW) {
    result.emplace(entryFromDbRow(stmt));
  }

  if (sqlite3_step(stmt) == SQLITE_DONE) {
    // TODO: log the error (opposite) somehow
  }

  sqlite3_finalize(stmt);

  return result;
}

void LibraryManager::addWatchedRomDirectory(
    const std::filesystem::path &romPath) {
  // TODO: Validations :-)
  watchedRomDirs.push_back(romPath);
}

bool LibraryManager::contains(int gameId) {
  // return std::any_of(entries.begin(), entries.end(),
  //                    [&gameId](const Entry &e) { return e.game == gameId; });
  return false;
}
std::vector<Entry> LibraryManager::getEntries() { return entries; }

void LibraryManager::insertEntry(Entry &entry) {
  const auto db = getOrCreateDbConnection();
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

  if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
    printf("prepare didn't work: %s\n", sqlite3_errmsg(database));
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
    printf("insert didn't work: %s\n", sqlite3_errmsg(database));
    // Handle error
  } else {
    if (sqlite3_changes(db) == 0) {
      // No new row was inserted, need to fetch the existing row's ID
      sqlite3_stmt *selectStmt = nullptr;
      const char *selectQuery = "SELECT id FROM library WHERE md5 = ?";

      if (sqlite3_prepare_v2(db, selectQuery, -1, &selectStmt, nullptr) ==
          SQLITE_OK) {
        sqlite3_bind_text(selectStmt, 1, entry.md5.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(selectStmt) == SQLITE_ROW) {
          entry.id = sqlite3_column_int(selectStmt, 0);
        } else {
          printf("Failed to retrieve existing row ID: %s\n",
                 sqlite3_errmsg(database));
          // Handle error
        }
        sqlite3_finalize(selectStmt);
      } else {
        printf("prepare for select didn't work: %s\n",
               sqlite3_errmsg(database));
        // Handle error
      }
    } else {
      // A new row was inserted
      entry.id = static_cast<int>(sqlite3_last_insert_rowid(db));
    }
  }

  // TODO: Do another query here to get the ID of the one we just put in...

  // Finalize the prepared statement to avoid resource leaks
  sqlite3_finalize(stmt);
  entries.push_back(entry);
}

sqlite3 *LibraryManager::getOrCreateDbConnection() const {
  if (database) {
    return database;
  }

  return nullptr;
}

//    "CREATE TABLE IF NOT EXISTS library("
// "id INTEGER PRIMARY KEY AUTOINCREMENT,"
// "display_name NVARCHAR(999) NOT NULL,"
// "verified INTEGER,"
// "platform NVARCHAR(999) NOT NULL,"
// "md5 NVARCHAR(999) NOT NULL UNIQUE,"
// "game INTEGER,"
// "rom INTEGER,"
// "romhack INTEGER,"
// "content_path NVARCHAR(999) NOT NULL);"
// "CREATE UNIQUE INDEX IF NOT EXISTS idx_md5 ON library (md5);";

Entry LibraryManager::entryFromDbRow(sqlite3_stmt *stmt) {
  int id = sqlite3_column_int(stmt, 0);

  std::string display_name = reinterpret_cast<char *>(
      const_cast<unsigned char *>(sqlite3_column_text(stmt, 1)));

  const int verified = sqlite3_column_int(stmt, 2);

  std::string platform = reinterpret_cast<char *>(
      const_cast<unsigned char *>(sqlite3_column_text(stmt, 3)));

  std::string md5 = reinterpret_cast<char *>(
      const_cast<unsigned char *>(sqlite3_column_text(stmt, 4)));

  const int game = sqlite3_column_int(stmt, 5);
  const int rom = sqlite3_column_int(stmt, 6);
  const int romhack = sqlite3_column_int(stmt, 7);

  std::string content_path = reinterpret_cast<char *>(
      const_cast<unsigned char *>(sqlite3_column_text(stmt, 8)));

  return {.id = id,
          .display_name = display_name,
          .verified = static_cast<bool>(verified),
          .platform = platform,
          .md5 = md5,
          .game = game,
          .rom = rom,
          .romhack = romhack,
          .content_path = content_path};
}
} // namespace FL::Library

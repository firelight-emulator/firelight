//
// Created by alexs on 12/13/2023.
//

#include "library_manager.hpp"
#include <fstream>
#include <iostream>
#include <openssl/evp.h>
#include <sqlite3.h>
#include <utility>
#include <vector>

namespace FL::Library {
const int MAX_FILESIZE_BYTES = 50000000;

const char *createQuery = "CREATE TABLE IF NOT EXISTS library("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                          "display_name TEXT NOT NULL,"
                          "verified INTEGER,"
                          "platform TEXT NOT NULL,"
                          "md5 TEXT NOT NULL UNIQUE,"
                          "game INTEGER,"
                          "rom INTEGER,"
                          "romhack INTEGER,"
                          "content_path TEXT NOT NULL);";

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

  romFileExtensions[".gb"] = "gameboy";
  romFileExtensions[".gbc"] = "gameboy color";
  romFileExtensions[".gba"] = "gameboy";
  romFileExtensions[".z64"] = "gameboy";
  romFileExtensions[".smc"] = "gameboy";
  addWatchedRomDirectory(defaultRomPath);

  // Open a new SQLite database (or create if not exists)
  int rc = sqlite3_open(libraryDbFile.string().c_str(), &database);

  if (rc) {
    std::cerr << "Cannot open database: " << sqlite3_errmsg(database)
              << std::endl;
    return;
  } else {
    std::cout << "Database opened successfully!" << std::endl;
  }

  // Execute the SQL statement
  rc = sqlite3_exec(database, createQuery, nullptr, nullptr, nullptr);

  if (rc != SQLITE_OK) {
    std::cerr << "SQL error: " << sqlite3_errmsg(database) << std::endl;
    return;
  } else {
    std::cout << "Table created successfully!" << std::endl;
  }

  // Query data from the table
  const char *selectSQL = "SELECT * FROM library;";

  //**********************************************
  sqlite3_stmt *stmt;
  rc = sqlite3_prepare_v2(database, selectSQL, -1, &stmt, nullptr);

  if (rc != SQLITE_OK) {
    std::cerr << "SQL error: " << sqlite3_errmsg(database) << std::endl;
    return;
  }

  // Iterate through the result set
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    int id = sqlite3_column_int(stmt, 0);
    const unsigned char *displayName = sqlite3_column_text(stmt, 1);
    const unsigned char *md5 = sqlite3_column_text(stmt, 2);
    const unsigned char *filename = sqlite3_column_text(stmt, 3);

    printf("ID: %d, display name: %s, filename: %s, md5: %s\n", id, displayName,
           filename, md5);
  }
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
      if (romFileExtensions.find(ext.string()) == romFileExtensions.end()) {
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
        // TODO: Will need to do a game lookup here as well.
        insertEntry({.id = -1,
                     .display_name = "default display name",
                     .verified = true,
                     .platform = result->platform,
                     .md5 = md5,
                     .game = result->game,
                     .rom = result->id,
                     .romhack = -1,
                     .content_path = entry.path().string()});
      } else {
        printf("DIDN'T WORK\n");
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

void LibraryManager::addWatchedRomDirectory(
    const std::filesystem::path &romPath) {
  // TODO: Validations :-)
  watchedRomDirs.push_back(romPath);
}

std::weak_ptr<Entry> LibraryManager::getByRomId(std::string romId) {
  return {};
}

bool LibraryManager::contains(int gameId) {
  // return std::any_of(entries.begin(), entries.end(),
  //                    [&gameId](const Entry &e) { return e.game == gameId; });
  return false;
}

void LibraryManager::insertEntry(const Entry &entry) {
  auto db = getOrCreateDbConnection();
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
  }

  // Finalize the prepared statement to avoid resource leaks
  sqlite3_finalize(stmt);
}

sqlite3 *LibraryManager::getOrCreateDbConnection() {
  if (database) {
    return database;
  }

  return nullptr;
}
} // namespace FL::Library

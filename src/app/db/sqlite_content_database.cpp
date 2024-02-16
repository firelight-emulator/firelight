//
// Created by alexs on 12/22/2023.
//

#include "sqlite_content_database.hpp"

#include "daos/game.hpp"

#include <iostream>
#include <utility>

SqliteContentDatabase::SqliteContentDatabase(std::filesystem::path dbFile)
    : databaseFile(std::move(dbFile)) {
  // TODO: throw error
  getOrCreateDbConnection();
}

std::optional<ROM> SqliteContentDatabase::getRomByMd5(const std::string &md5) {
  sqlite3_stmt *stmt = nullptr;
  auto query = "SELECT * FROM roms WHERE md5 = ?";

  if (sqlite3_prepare_v2(database, query, -1, &stmt, nullptr) != SQLITE_OK) {
    printf("prepare didn't work\n");
    // TODO: error handle
    sqlite3_finalize(stmt);
    return {};
  }

  if (sqlite3_bind_text(stmt, 1, md5.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
    printf("bind didn't work: %s\n", sqlite3_errmsg(database));
    // TODO: Handle error
    sqlite3_finalize(stmt);
    return {};
  }

  std::optional<ROM> result;

  if (sqlite3_step(stmt) == SQLITE_ROW) {
    result.emplace(romFromDbRow(stmt));
  }

  if (sqlite3_step(stmt) == SQLITE_DONE) {
    // TODO: log the error (opposite) somehow
  }

  sqlite3_finalize(stmt);

  return result;
}
std::optional<Game> SqliteContentDatabase::getGameByRomId(int romId) {
  sqlite3_stmt *stmt = nullptr;
  const auto query = "SELECT games.* FROM roms INNER JOIN games ON games.id = "
                     "roms.game WHERE roms.id = ?";

  if (sqlite3_prepare_v2(database, query, -1, &stmt, nullptr) != SQLITE_OK) {
    printf("prepare didn't work\n");
    // TODO: error handle
    sqlite3_finalize(stmt);
    return {};
  }

  if (sqlite3_bind_int(stmt, 1, romId) != SQLITE_OK) {
    printf("bind didn't work: %s\n", sqlite3_errmsg(database));
    // TODO: Handle error
    sqlite3_finalize(stmt);
    return {};
  }

  std::optional<Game> result;

  if (sqlite3_step(stmt) == SQLITE_ROW) {
    result.emplace(gameFromDbRow(stmt));
  }

  if (sqlite3_step(stmt) == SQLITE_DONE) {
    // TODO: log the error (opposite) somehow
  }

  sqlite3_finalize(stmt);

  return result;
}
std::optional<Romhack>
SqliteContentDatabase::getRomhackByMd5(const std::string &md5) {
  std::optional<Romhack> result;

  return result;
}

std::optional<Platform>
SqliteContentDatabase::getPlatformByExtension(std::string ext) {
  std::optional<Platform> result;

  if (ext == ".n64" || ext == ".v64" || ext == ".z64") {
    result.emplace(Platform{.id = 0});
  } else if (ext == ".smc") {
    result.emplace(Platform{.id = 1});
  } else if (ext == ".gbc") {
    result.emplace(Platform{.id = 2});
  } else if (ext == ".gb") {
    result.emplace(Platform{.id = 3});
  }
  // else if (ext == ".gba") {
  //   result.emplace(Platform{.id = 4});
  // }

  return result;
}

sqlite3 *SqliteContentDatabase::getOrCreateDbConnection() {
  if (sqlite3_open(databaseFile.string().c_str(), &database)) {
    std::cerr << "Cannot open database: " << sqlite3_errmsg(database)
              << std::endl;
    return nullptr;
  }

  return database;
}
ROM SqliteContentDatabase::romFromDbRow(sqlite3_stmt *stmt) {
  int id = sqlite3_column_int(stmt, 0);
  std::string filename = reinterpret_cast<char *>(
      const_cast<unsigned char *>(sqlite3_column_text(stmt, 1)));
  std::string file_ext = reinterpret_cast<char *>(
      const_cast<unsigned char *>(sqlite3_column_text(stmt, 2)));
  const int game_id = sqlite3_column_int(stmt, 3);
  std::string platform = reinterpret_cast<char *>(
      const_cast<unsigned char *>(sqlite3_column_text(stmt, 4)));
  std::string region = reinterpret_cast<char *>(
      const_cast<unsigned char *>(sqlite3_column_text(stmt, 5)));
  std::string rom_md5 = reinterpret_cast<char *>(
      const_cast<unsigned char *>(sqlite3_column_text(stmt, 6)));
  const int size_bytes = sqlite3_column_int(stmt, 7);

  return {.id = id,
          .filename = filename,
          .file_ext = file_ext,
          .game = game_id,
          .platform = platform,
          .region = region,
          .md5 = rom_md5,
          .size_bytes = size_bytes};
}
Game SqliteContentDatabase::gameFromDbRow(sqlite3_stmt *stmt) {
  int id = sqlite3_column_int(stmt, 0);
  std::string name = reinterpret_cast<char *>(
      const_cast<unsigned char *>(sqlite3_column_text(stmt, 1)));

  return {.id = id, .name = name};
}

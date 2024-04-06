//
// Created by alexs on 12/22/2023.
//

#include "sqlite_content_database.hpp"
#include "firelight/game.hpp"

#include <iostream>
#include <utility>

const std::string radicalRedDescription =
    "<p>Pokémon Radical Red is an enhancement hack of Pokémon Fire Red.</p>"
    "<p>This is a difficulty hack, with massive additional features added to "
    "help you navigate through this game.</p>"
    " <p>This hack utilises the Complete Fire Red Upgrade Engine and Dynamic "
    "Pokemon Expansion built"
    "by Skeli789, Ghoulslash, and others. It's responsible for most of the "
    "significant features"
    "in the hack.</p>"
    "<p>List of features (Most of them provided by CFRU and DPE):</p>"
    "<ul>"
    "    <li>Much higher difficulty, with optional modes to add or mitigate "
    "difficulty</li>"
    "    <li>Built-in Randomizer options (Pokémon, Abilities and "
    "Learnsets)</li>"
    "    <li>Physical/Special split + Fairy Typing</li>"
    "    <li>All Pokémon up to Gen 9 obtainable (with some exceptions)</li>"
    "    <li>Most Moves up to Gen 9</li>"
    "    <li>Updated Pokémon sprites</li>"
    "    <li>Mega Evolutions & Z-Moves</li>"
    "    <li>Most Abilities up to Gen 9</li>"
    "    <li>All important battle items (with some exceptions)</li>"
    "    <li>Wish Piece Raid Battles (with Dynamax)</li>"
    "    <li>Mystery Gifts</li>"
    "    <li>Reusable TMs</li>"
    "    <li>Expanded TM list</li>"
    "    <li>Additional move tutors</li>"
    "    <li>EV Training Gear and NPCs</li>"
    "    <li>Ability popups during battle</li>"
    "    <li>Party Exp Share (can be disabled)</li>"
    "    <li>Hidden Abilities</li>"
    "    <li>Day, Dusk and Night cycle (syncs with RTC)</li>"
    "    <li>DexNav, which allows you to search for Pokémon with hidden "
    "abilities and more</li>"
    "    <li>Even faster turbo speed on bike and while surfing</li>"
    "    <li>Abilities like Magma Armor, Static, or Flash Fire have overworld "
    "effects like in recent generations</li>"
    "    <li>Destiny Knot, Everstone have updated breeding mechanics</li>"
    "    <li>Lots of Quality of Life changes</li>"
    "    <li>... and more!</li>"
    "</ul>";

const std::string goombossDescription =
    "<p>Saving the world is hungry work. With the Star Rod recovered, a "
    "satisfied Mario and Goombario head out for a bite to eat. But who's "
    "this in their favourite picnic spot? And more importantly, can you come "
    "up with a strategy to defeat...</p>"
    "<br />"
    "<p>THE ULTIMATE GOOMBOSS CHALLENGE?</p>"
    "<br />"
    "<p>Made as a love letter to RPG boss battles, and a determination to make "
    "the most epic possible final product using only Goombas.</p>"
    "<br />"
    "<p>WARNING: May have bugs.</p>";

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
  } else if (ext == ".gba") {
    result.emplace(Platform{.id = 4});
  } else if (ext == ".nds") {
    result.emplace(Platform{.id = 5});
  } else if (ext == ".md") {
    result.emplace(Platform{.id = 6});
  }

  return result;
}

bool SqliteContentDatabase::tableExists(const std::string &tableName) {
  return true;
}

std::optional<ROM> SqliteContentDatabase::getRom(const int id) {
  sqlite3_stmt *stmt = nullptr;
  auto query = "SELECT * FROM roms WHERE id = ?";

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
std::optional<firelight::db::Mod> SqliteContentDatabase::getMod(int id) {
  return {};
}
std::optional<firelight::db::ModRelease>
SqliteContentDatabase::getModRelease(int id) {
  return {};
}
std::optional<firelight::db::Patch> SqliteContentDatabase::getPatch(int id) {
  return {};
}
std::optional<Platform> SqliteContentDatabase::getPlatform(int id) {
  switch (id) {
  case 0:
    return {{0, "Game Boy Advance", "GBA"}};
  case 1:
    return {{1, "Nintendo 64", "N64"}};
  default:
    return {};
  }
}
std::optional<firelight::db::Region> SqliteContentDatabase::getRegion(int id) {
  return {};
}

std::optional<firelight::db::Game>
SqliteContentDatabase::getGame(const int id) {
  switch (id) {
  case 0:
    return {{0, "Pokemon FireRed Version", "TBD"}};
  case 1:
    return {{1, "Paper Mario", "TBD"}};
  default:
    return {};
  }
}

std::optional<firelight::db::GameRelease>
SqliteContentDatabase::getGameRelease(const int id) {
  switch (id) {
  case 0:
    return {{0, 0, 0, -1, {}}};
  case 1:
    return {{1, 1, 1, -1, {356}}};
  default:
    return {};
  }
}
std::vector<firelight::db::Mod> SqliteContentDatabase::getAllMods() {
  return {{0,
           "Pokemon Radical Red",
           "soupercell",
           {0},
           "file:///Users/alexs/git/firelight/build/pkmnrr.png",
           radicalRedDescription},
          {1,
           "Ultimate Goomboss Challenge",
           "Enneagon",
           {1},
           "file:///Users/alexs/git/firelight/build/ultimategoomboss.png",
           goombossDescription}};
}
std::vector<ROM> SqliteContentDatabase::getMatchingRoms(const ROM &rom) {
  return {};
}
std::vector<firelight::db::Patch>
SqliteContentDatabase::getMatchingPatches(const firelight::db::Patch &patch) {
  return {};
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

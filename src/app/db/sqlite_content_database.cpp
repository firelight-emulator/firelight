#include "sqlite_content_database.hpp"
#include "firelight/db/game.hpp"

#include <QSqlError>
#include <QSqlQuery>
#include <QThread>
#include <spdlog/spdlog.h>
#include <utility>

namespace firelight::db {

constexpr auto DATABASE_PREFIX = "content_";

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
  // TODO Test DB connection or w/e
}

SqliteContentDatabase::~SqliteContentDatabase() {
  for (const auto &name : QSqlDatabase::connectionNames()) {
    if (name.startsWith(DATABASE_PREFIX)) {
      QSqlDatabase::removeDatabase(name);
    }
  }
}
// std::optional<Platform>
// SqliteContentDatabase::getPlatformByExtension(std::string ext) {
//   std::optional<Platform> result;
//
//   if (ext == ".n64" || ext == ".v64" || ext == ".z64") {
//     result.emplace(Platform{.id = 0});
//   } else if (ext == ".smc") {
//     result.emplace(Platform{.id = 1});
//   } else if (ext == ".gbc") {
//     result.emplace(Platform{.id = 2});
//   } else if (ext == ".gb") {
//     result.emplace(Platform{.id = 3});
//   } else if (ext == ".gba") {
//     result.emplace(Platform{.id = 4});
//   } else if (ext == ".nds") {
//     result.emplace(Platform{.id = 5});
//   } else if (ext == ".md") {
//     result.emplace(Platform{.id = 6});
//   }
//
//   return result;
// }

bool SqliteContentDatabase::tableExists(const std::string &tableName) {
  return true;
}

std::optional<Game> SqliteContentDatabase::getGame(const int id) {
  QSqlQuery query(getDatabase());
  query.prepare("SELECT * FROM games WHERE id = :id");
  query.bindValue(":id", id);

  if (!query.exec()) {
    spdlog::error("Failed to get game: {}",
                  query.lastError().text().toStdString());
    return std::nullopt;
  }

  if (!query.next()) {
    return std::nullopt;
  }

  return createGameFromQuery(query);
}

std::optional<ROM> SqliteContentDatabase::getRom(const int id) {
  QSqlQuery query(getDatabase());
  query.prepare("SELECT * FROM roms WHERE id = :id");
  query.bindValue(":id", id);

  if (!query.exec()) {
    spdlog::error("Failed to get rom: {}",
                  query.lastError().text().toStdString());
    return std::nullopt;
  }

  if (!query.next()) {
    return std::nullopt;
  }

  return createRomFromQuery(query);
}

std::vector<ROM> SqliteContentDatabase::getMatchingRoms(const ROM &rom) {
  QString queryString = "SELECT * FROM roms";

  QString whereClause;
  bool needAND = false;

  if (rom.id != -1) {
    whereClause += " WHERE id = :id";
    needAND = true;
  }

  if (!rom.md5.empty()) {
    if (needAND) {
      whereClause += " AND ";
    } else {
      whereClause += " WHERE ";
      needAND = true;
    }
    whereClause += "md5 = :md5";
  }

  queryString += whereClause;
  QSqlQuery query(getDatabase());
  query.prepare(queryString);

  if (rom.id != -1) {
    query.bindValue(":id", rom.id);
  }
  if (!rom.md5.empty()) {
    query.bindValue(":md5", QString::fromStdString(rom.md5));
  }

  if (!query.exec()) {
    spdlog::error("ruh roh raggy: {}", query.lastError().text().toStdString());
  }

  std::vector<ROM> roms;

  while (query.next()) {
    roms.emplace_back(createRomFromQuery(query));
  }

  return roms;
}

std::optional<Mod> SqliteContentDatabase::getMod(const int id) {
  QSqlQuery query(getDatabase());
  query.prepare("SELECT * FROM mods WHERE id = :id");
  query.bindValue(":id", id);

  if (!query.exec()) {
    spdlog::error("Failed to get mod: {}",
                  query.lastError().text().toStdString());
    return std::nullopt;
  }

  if (!query.next()) {
    return std::nullopt;
  }

  return createModFromQuery(query);
}

std::vector<Mod> SqliteContentDatabase::getAllMods() {
  // return {{0, "Pokemon Radical Red", "soupercell", 0,
  //          "file:///Users/alexs/git/firelight/build/pkmnrr.png",
  //          radicalRedDescription},
  //         {1, "Ultimate Goomboss Challenge", "Enneagon", 1,
  //          "file:///Users/alexs/git/firelight/build/ultimategoomboss.png",
  //          goombossDescription}};
  return {};
}

std::optional<Patch> SqliteContentDatabase::getPatch(const int id) {
  QSqlQuery query(getDatabase());
  query.prepare("SELECT * FROM patches WHERE id = :id");
  query.bindValue(":id", id);

  if (!query.exec()) {
    spdlog::error("Failed to get patch: {}",
                  query.lastError().text().toStdString());
    return std::nullopt;
  }

  if (!query.next()) {
    return std::nullopt;
  }

  return createPatchFromQuery(query);
}

std::vector<Patch>
SqliteContentDatabase::getMatchingPatches(const Patch &patch) {
  return {};
}

std::optional<Platform> SqliteContentDatabase::getPlatform(const int id) {
  QSqlQuery query(getDatabase());
  query.prepare("SELECT * FROM platforms WHERE id = :id");
  query.bindValue(":id", id);

  if (!query.exec()) {
    spdlog::error("Failed to get platform: {}",
                  query.lastError().text().toStdString());
    return std::nullopt;
  }

  if (!query.next()) {
    return std::nullopt;
  }

  return createPlatformFromQuery(query);
}

std::vector<Platform>
SqliteContentDatabase::getMatchingPlatforms(const Platform &platform) {
  const auto ext = platform.supportedExtensions.at(0);

  if (ext == ".n64" || ext == ".v64" || ext == ".z64") {
    return {Platform{.id = 0}};
  }
  if (ext == ".smc" || ext == ".sfc") {
    return {Platform{.id = 1}};
  }
  if (ext == ".gbc") {
    return {Platform{.id = 2}};
  }
  if (ext == ".gb") {
    return {Platform{.id = 3}};
  }
  if (ext == ".gba") {
    return {Platform{.id = 4}};
  }
  if (ext == ".nds") {
    return {Platform{.id = 5}};
  }

  if (ext == ".md") {
    return {Platform{.id = 6}};
  }

  return {};
}

std::optional<Region> SqliteContentDatabase::getRegion(const int id) {
  QSqlQuery query(getDatabase());
  query.prepare("SELECT * FROM regions WHERE id = :id");
  query.bindValue(":id", id);

  if (!query.exec()) {
    spdlog::error("Failed to get region: {}",
                  query.lastError().text().toStdString());
    return std::nullopt;
  }

  if (!query.next()) {
    return std::nullopt;
  }

  return createRegionFromQuery(query);
}

QSqlDatabase SqliteContentDatabase::getDatabase() const {
  const auto name =
      DATABASE_PREFIX +
      QString::number(reinterpret_cast<quint64>(QThread::currentThread()), 16);
  if (QSqlDatabase::contains(name)) {
    return QSqlDatabase::database(name);
  }

  spdlog::debug("Database connection with name {} does not exist; creating",
                name.toStdString());
  auto db = QSqlDatabase::addDatabase("QSQLITE", name);
  db.setDatabaseName(QString::fromStdString(databaseFile.string()));
  db.open();
  return db;
}

Game SqliteContentDatabase::createGameFromQuery(const QSqlQuery &query) {
  Game game;
  game.id = query.value(0).toInt();
  game.name = query.value(1).toString().toStdString();
  game.slug = query.value(2).toString().toStdString();
  game.description = query.value(3).toString().toStdString();
  game.platformId = query.value(4).toInt();

  return game;
}

Mod SqliteContentDatabase::createModFromQuery(const QSqlQuery &query) {
  return {};
}
ROM SqliteContentDatabase::createRomFromQuery(const QSqlQuery &query) {
  ROM rom;
  rom.id = query.value(0).toInt();
  rom.filename = query.value(1).toString().toStdString();
  rom.size_bytes = query.value(2).toInt();
  rom.gameId = query.value(3).toInt();
  rom.gameIdVerified = query.value(4).toBool();
  rom.platformId = query.value(5).toInt();
  rom.md5 = query.value(6).toString().toStdString();
  rom.sha1 = query.value(7).toString().toStdString();
  rom.crc32 = query.value(8).toString().toStdString();

  return rom;
}
Patch SqliteContentDatabase::createPatchFromQuery(const QSqlQuery &query) {
  return {};
}
Platform
SqliteContentDatabase::createPlatformFromQuery(const QSqlQuery &query) {
  return {};
}
Region SqliteContentDatabase::createRegionFromQuery(const QSqlQuery &query) {
  return {};
}

} // namespace firelight::db

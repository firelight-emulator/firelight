#include "sqlite_content_database.hpp"
#include "firelight/db/game.hpp"

#include <QSqlError>
#include <QSqlQuery>
#include <QThread>
#include <spdlog/spdlog.h>
#include <utility>

namespace firelight::db {
  constexpr auto DATABASE_PREFIX = "content_";

  SqliteContentDatabase::SqliteContentDatabase(std::filesystem::path dbFile)
    : databaseFile(std::move(dbFile)) {
    // TODO Test DB connection or w/e
  }

  SqliteContentDatabase::~SqliteContentDatabase() {
    for (const auto &name: QSqlDatabase::connectionNames()) {
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
    QSqlQuery query(getDatabase());
    query.prepare("SELECT 1 FROM " + QString::fromStdString(tableName) +
                  " LIMIT 1;");

    return query.exec();
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

  std::optional<int> SqliteContentDatabase::getRetroAchievementsIdForGame(const int id) {
    QSqlQuery query(getDatabase());
    query.prepare(
      "SELECT external_id FROM game_external_ids WHERE game_id = :id AND external_system_name = 'retroachievements'");
    query.bindValue(":id", id);

    if (!query.exec()) {
      spdlog::error("Failed to get external id: {}",
                    query.lastError().text().toStdString());
      return std::nullopt;
    }

    if (!query.next()) {
      return std::nullopt;
    }

    return query.value(0).toInt();
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
    QSqlQuery query(getDatabase());
    query.prepare("SELECT * FROM mods");

    if (!query.exec()) {
      spdlog::error("Failed to get mods: {}",
                    query.lastError().text().toStdString());
      return {};
    }

    std::vector<Mod> mods;

    while (query.next()) {
      mods.emplace_back(createModFromQuery(query));
    }

    return mods;
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
    QString queryString = "SELECT * FROM patches";

    QString whereClause;
    bool needAND = false;

    if (patch.id != -1) {
      whereClause += " WHERE id = :id";
      needAND = true;
    }

    if (!patch.md5.empty()) {
      if (needAND) {
        whereClause += " AND ";
      } else {
        whereClause += " WHERE ";
        needAND = true;
      }
      whereClause += "md5 = :md5";
    }

    if (patch.modId != -1) {
      if (needAND) {
        whereClause += " AND ";
      } else {
        whereClause += " WHERE ";
        needAND = true;
      }
      whereClause += "mod_id = :modId";
    }

    queryString += whereClause;
    QSqlQuery query(getDatabase());
    query.prepare(queryString);

    if (patch.id != -1) {
      query.bindValue(":id", patch.id);
    }
    if (!patch.md5.empty()) {
      query.bindValue(":md5", QString::fromStdString(patch.md5));
    }
    if (patch.modId != -1) {
      query.bindValue(":modId", patch.modId);
    }

    if (!query.exec()) {
      spdlog::error("Failed to get matching patches: {}",
                    query.lastError().text().toStdString());
    }

    std::vector<Patch> patches;

    while (query.next()) {
      patches.emplace_back(createPatchFromQuery(query));
    }

    return patches;
  }

  std::optional<Platform> SqliteContentDatabase::getPlatform(const int id) {
    QSqlQuery query(getDatabase());
    query.prepare(
      "SELECT platforms.id, platforms.name, platforms.abbreviation, "
      "platforms.slug, platform_external_ids.external_id FROM platforms "
      "JOIN platform_external_ids ON platforms.id = "
      "platform_external_ids.platform_id "
      "AND platform_external_ids.external_system_name = retroachievements "
      "WHERE id = :id");
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
      return {Platform{.id = 7}};
    }
    if (ext == ".smc" || ext == ".sfc") {
      return {Platform{.id = 6}};
    }
    if (ext == ".gbc") {
      return {Platform{.id = 2}};
    }
    if (ext == ".gb") {
      return {Platform{.id = 1}};
    }
    if (ext == ".gba") {
      return {Platform{.id = 3}};
    }
    if (ext == ".nds") {
      return {Platform{.id = 10}};
    }

    if (ext == ".md") {
      return {Platform{.id = 13}};
    }

    if (ext == ".nes") {
      return {Platform{.id = 5}};
    }

    if (ext == ".iso") {
      return {Platform{.id = 20}};
    }

    if (ext == ".sms") {
      return {Platform{.id = 12}};
    }

    if (ext == ".gg") {
      return {Platform{.id = 14}};
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
    Mod mod;
    mod.id = query.value(0).toInt();
    mod.name = query.value(1).toString().toStdString();
    mod.slug = query.value(2).toString().toStdString();
    mod.gameId = query.value(3).toInt();
    mod.description = query.value(4).toString().toStdString();
    mod.imageSource = query.value(5).toString().toStdString();
    mod.primaryAuthor = query.value(6).toString().toStdString();

    return mod;
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
    Patch patch;
    patch.id = query.value(0).toInt();
    patch.version = query.value(1).toString().toStdString();
    patch.filename = query.value(2).toString().toStdString();
    patch.sizeBytes = query.value(3).toInt();
    patch.modId = query.value(4).toInt();
    patch.romId = query.value(5).toInt();
    patch.md5 = query.value(6).toString().toStdString();
    patch.sha1 = query.value(7).toString().toStdString();
    patch.crc32 = query.value(8).toString().toStdString();

    return patch;
  }

  Platform
  SqliteContentDatabase::createPlatformFromQuery(const QSqlQuery &query) {
    Platform platform;
    platform.id = query.value(0).toInt();
    platform.name = query.value(1).toString().toStdString();
    platform.abbreviation = query.value(2).toString().toStdString();
    platform.slug = query.value(3).toString().toStdString();

    return platform;
  }

  Region SqliteContentDatabase::createRegionFromQuery(const QSqlQuery &query) {
    Region region;
    region.id = query.value(0).toInt();
    region.name = query.value(1).toString().toStdString();
    region.abbreviation = query.value(2).toString().toStdString();
    region.slug = query.value(3).toString().toStdString();

    return region;
  }
} // namespace firelight::db

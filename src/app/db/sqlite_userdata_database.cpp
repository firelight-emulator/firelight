#include "sqlite_userdata_database.hpp"

#include <QSqlError>
#include <QSqlQuery>
#include <spdlog/spdlog.h>

#include "../saves/suspend_point.hpp"

namespace firelight::db {
  constexpr auto DATABASE_PREFIX = "userdata_";

  SqliteUserdataDatabase::SqliteUserdataDatabase(
    const std::filesystem::path &dbFile)
    : m_database_path(dbFile) {
    m_database = QSqlDatabase::addDatabase("QSQLITE", "userdata");
    m_database.setDatabaseName(QString::fromStdString(dbFile.string()));
    if (!m_database.open()) {
      throw std::runtime_error("Couldn't open Userdata database");
    }

    QSqlQuery createSavefileMetadata(m_database);
    createSavefileMetadata.prepare("CREATE TABLE IF NOT EXISTS savefile_metadata("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "content_id TEXT NOT NULL,"
      "slot_number INTEGER NOT NULL,"
      "savefile_md5 TEXT NOT NULL,"
      "last_modified_at INTEGER NOT NULL,"
      "created_at INTEGER NOT NULL, "
      "UNIQUE(content_id, slot_number));");

    if (!createSavefileMetadata.exec()) {
      spdlog::error("Table creation failed: {}",
                    createSavefileMetadata.lastError().text().toStdString());
    }

    QSqlQuery createSuspendPointMetadata(m_database);
    createSuspendPointMetadata.prepare("CREATE TABLE IF NOT EXISTS suspend_point_metadata("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "content_id TEXT NOT NULL,"
      "save_slot_number INTEGER NOT NULL,"
      "slot_number INTEGER NOT NULL,"
      "locked INTEGER NOT NULL DEFAULT 0,"
      "last_modified_at INTEGER NOT NULL,"
      "created_at INTEGER NOT NULL, "
      "UNIQUE(content_id, slot_number));");

    if (!createSuspendPointMetadata.exec()) {
      spdlog::error("Table creation failed: {}",
                    createSuspendPointMetadata.lastError().text().toStdString());
    }

    QSqlQuery createPlaySessions(m_database);
    createPlaySessions.prepare("CREATE TABLE IF NOT EXISTS play_sessions("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "content_id TEXT NOT NULL,"
      "savefile_slot_number INTEGER NOT NULL,"
      "start_time INTEGER NOT NULL,"
      "end_time INTEGER NOT NULL,"
      "unpaused_duration_seconds INTEGER NOT NULL);");

    if (!createPlaySessions.exec()) {
      spdlog::error("Table creation failed: {}",
                    createPlaySessions.lastError().text().toStdString());
    }

    QSqlQuery createControllerProfiles(m_database);
    createControllerProfiles.prepare("CREATE TABLE IF NOT EXISTS controller_profiles("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "display_name TEXT NOT NULL);");

    if (!createControllerProfiles.exec()) {
      spdlog::error("Table creation failed: {}",
                    createControllerProfiles.lastError().text().toStdString());
    }

    QSqlQuery createInputMappings(m_database);
    createInputMappings.prepare("CREATE TABLE IF NOT EXISTS input_mappings("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "controller_profile_id INTEGER NOT NULL,"
      "platform_id INTEGER NOT NULL, "
      "UNIQUE(controller_profile_id, platform_id));");

    if (!createInputMappings.exec()) {
      spdlog::error("Table creation failed: {}",
                    createInputMappings.lastError().text().toStdString());
    }

    QSqlQuery createPlatformSettings(m_database);
    createPlatformSettings.prepare("CREATE TABLE IF NOT EXISTS platform_settings("
      "platform_id INTEGER NOT NULL, "
      "key TEXT NOT NULL,"
      "value TEXT NOT NULL,"
      "UNIQUE(platform_id, key));");

    if (!createPlatformSettings.exec()) {
      spdlog::error("Table creation failed: {}",
                    createPlatformSettings.lastError().text().toStdString());
    }
  }

  SqliteUserdataDatabase::~SqliteUserdataDatabase() {
    m_database.close();
    QSqlDatabase::removeDatabase(m_database.connectionName());
  }

  std::vector<SavefileMetadata>
  SqliteUserdataDatabase::getSavefileMetadataForContent(
    const std::string contentId) {
    QSqlQuery query(m_database);
    query.prepare(
      "SELECT * FROM savefile_metadata WHERE content_id = :contentId;");
    query.bindValue(":contentId", QString::fromStdString(contentId));

    if (!query.exec()) {
      spdlog::warn("Could not retrieve savefile metadata: {}",
                   query.lastError().text().toStdString());
      return {};
    }

    std::vector<SavefileMetadata> metadataList;
    while (query.next()) {
      SavefileMetadata metadata;
      metadata.id = query.value("id").toUInt();
      metadata.contentId = query.value("content_id").toString().toStdString();
      metadata.slotNumber = query.value("slot_number").toUInt();
      metadata.savefileMd5 = query.value("savefile_md5").toString().toStdString();
      metadata.lastModifiedAt = query.value("last_modified_at").toLongLong();
      metadata.createdAt = query.value("created_at").toLongLong();
      metadataList.emplace_back(metadata);
    }

    return metadataList;
  }

  bool SqliteUserdataDatabase::createSuspendPointMetadata(SuspendPointMetadata &metadata) {
    if (!m_database.open()) {
      spdlog::error("Couldn't open database: {}",
                    m_database.lastError().text().toStdString());
      return false;
    }

    const QString queryString = "INSERT INTO suspend_point_metadata (content_id, save_slot_number, "
        "slot_number, locked, last_modified_at, "
        "created_at) VALUES (:contentId, :slotNumber, "
        ":locked, :lastModifiedAt, :createdAt);";

    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch())
        .count();

    QSqlQuery query(m_database);
    query.prepare(queryString);
    query.bindValue(":contentId", QString::fromStdString(metadata.contentId));
    query.bindValue(":saveSlotNumber", metadata.saveSlotNumber);
    query.bindValue(":slotNumber", metadata.slotNumber);
    query.bindValue(":locked", metadata.locked);
    query.bindValue(":lastModifiedAt", timestamp);
    query.bindValue(":createdAt", timestamp);

    if (!query.exec()) {
      query.finish();
      return false;
    }

    metadata.id = query.lastInsertId().toInt();

    query.finish();
    return true;
  }

  std::optional<SuspendPointMetadata> SqliteUserdataDatabase::getSuspendPointMetadata(
    std::string contentId, int saveSlotNumber,
    int slotNumber) {
    const QString queryString =
        "SELECT * FROM suspend_point_metadata WHERE content_id = :contentId AND save_slot_number = :saveSlotNumber AND "
        "slot_number = :slotNumber LIMIT 1;";
    QSqlQuery query(m_database);
    query.prepare(queryString);
    query.bindValue(":contentId", QString::fromStdString(contentId));
    query.bindValue(":saveSlotNumber", saveSlotNumber);
    query.bindValue(":slotNumber", slotNumber);

    if (!query.exec()) {
      spdlog::error("Failed to get suspend point metadata: {}",
                    query.lastError().text().toStdString());
      return std::nullopt;
    }

    if (!query.next()) {
      return std::nullopt;
    }

    SuspendPointMetadata metadata;
    metadata.id = query.value("id").toUInt();
    metadata.contentId = query.value("content_id").toString().toStdString();
    metadata.saveSlotNumber = query.value("save_slot_number").toUInt();
    metadata.slotNumber = query.value("slot_number").toUInt();
    metadata.locked = query.value("savefile_md5").toBool();
    metadata.lastModifiedAt = query.value("last_modified_at").toLongLong();
    metadata.createdAt = query.value("created_at").toLongLong();

    return metadata;
  }

  bool SqliteUserdataDatabase::updateSuspendPointMetadata(SuspendPointMetadata metadata) {
    return false;
  }

  std::vector<SuspendPointMetadata> SqliteUserdataDatabase::getSuspendPointMetadataForContent(
    std::string contentId, int saveSlotNumber) {
    QSqlQuery query(m_database);
    query.prepare(
      "SELECT * FROM suspend_point_metadata WHERE content_id = :contentId AND save_slot_number = :saveSlotNumber;");
    query.bindValue(":contentId", QString::fromStdString(contentId));
    query.bindValue(":saveSlotNumber", saveSlotNumber);

    if (!query.exec()) {
      spdlog::warn("Could not retrieve savefile metadata: {}",
                   query.lastError().text().toStdString());
      return {};
    }

    std::vector<SuspendPointMetadata> metadataList;
    while (query.next()) {
      SuspendPointMetadata metadata;
      metadata.id = query.value("id").toUInt();
      metadata.contentId = query.value("content_id").toString().toStdString();
      metadata.saveSlotNumber = query.value("save_slot_number").toUInt();
      metadata.slotNumber = query.value("slot_number").toUInt();
      metadata.locked = query.value("savefile_md5").toBool();
      metadata.lastModifiedAt = query.value("last_modified_at").toLongLong();
      metadata.createdAt = query.value("created_at").toLongLong();
    }

    return metadataList;
  }

  bool SqliteUserdataDatabase::deleteSuspendPointMetadata(int id) {
    return true;
  }

  bool SqliteUserdataDatabase::tableExists(const std::string tableName) {
    QSqlQuery query(m_database);
    query.prepare("SELECT 1 FROM " + QString::fromStdString(tableName) +
                  " LIMIT 1;");

    return query.exec();
  }

  std::optional<SavefileMetadata>
  SqliteUserdataDatabase::getSavefileMetadata(const std::string contentId,
                                              const int slotNumber) {
    const QString queryString =
        "SELECT * FROM savefile_metadata WHERE content_id = :contentId AND "
        "slot_number = :slotNumber LIMIT 1;";
    QSqlQuery query(m_database);
    query.prepare(queryString);
    query.bindValue(":contentId", QString::fromStdString(contentId));
    query.bindValue(":slotNumber", slotNumber);

    if (!query.exec()) {
      spdlog::error("Failed to get savefile metadata: {}",
                    query.lastError().text().toStdString());
      return std::nullopt;
    }

    if (!query.next()) {
      return std::nullopt;
    }

    SavefileMetadata metadata;
    metadata.id = query.value("id").toUInt();
    metadata.contentId = query.value("content_id").toString().toStdString();
    metadata.slotNumber = query.value("slot_number").toUInt();
    metadata.savefileMd5 = query.value("savefile_md5").toString().toStdString();
    metadata.lastModifiedAt = query.value("last_modified_at").toLongLong();
    metadata.createdAt = query.value("created_at").toLongLong();

    return metadata;
  }

  bool SqliteUserdataDatabase::updateSavefileMetadata(SavefileMetadata metadata) {
    QSqlQuery query(m_database);
    query.prepare("UPDATE savefile_metadata SET savefile_md5 = :savefileMd5, "
      "last_modified_at = :lastModifiedAt WHERE id = :id;");
    query.bindValue(":savefileMd5", QString::fromStdString(metadata.savefileMd5));
    query.bindValue(":lastModifiedAt", metadata.lastModifiedAt);
    query.bindValue(":id", metadata.id);

    if (!query.exec()) {
      spdlog::error("Update Savefile metadata failed: {}",
                    query.lastError().text().toStdString());
      return false;
    }

    return query.numRowsAffected() >= 1;
  }

  bool SqliteUserdataDatabase::createSavefileMetadata(
    SavefileMetadata &metadata) {
    if (!m_database.open()) {
      spdlog::error("Couldn't open database: {}",
                    m_database.lastError().text().toStdString());
      return false;
    }

    const QString queryString = "INSERT INTO savefile_metadata (content_id, "
        "slot_number, savefile_md5, last_modified_at, "
        "created_at) VALUES (:contentId, :slotNumber, "
        ":savefileMd5, :lastModifiedAt, :createdAt);";

    QSqlQuery query(m_database);
    query.prepare(queryString);
    query.bindValue(":contentId", QString::fromStdString(metadata.contentId));
    query.bindValue(":slotNumber", metadata.slotNumber);
    query.bindValue(":savefileMd5", QString::fromStdString(metadata.savefileMd5));
    query.bindValue(":lastModifiedAt", metadata.lastModifiedAt);
    query.bindValue(":createdAt",
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::system_clock::now().time_since_epoch())
                    .count());

    if (!query.exec()) {
      query.finish();
      return false;
    }

    metadata.id = query.lastInsertId().toInt();

    query.finish();
    return true;
  }


  bool SqliteUserdataDatabase::createControllerProfile(ControllerProfile &profile) {
    return false;
  }

  std::optional<ControllerProfile> SqliteUserdataDatabase::getControllerProfile(const int id) {
    printf("Getting controller profile: %d\n", id);
    return {
      {
        id,
        "Test",
        {
          {
            0, id, 1
          }
        }
      }
    };
  }

  std::vector<ControllerProfile> SqliteUserdataDatabase::getControllerProfiles() {
    return {};
  }

  bool SqliteUserdataDatabase::createPlaySession(PlaySession &session) {
    const QString queryString = "INSERT INTO play_sessions ("
        "content_id, "
        "savefile_slot_number, "
        "start_time, "
        "end_time, "
        "unpaused_duration_seconds) "
        "VALUES (:contentId, :slotNumber, :startTime,"
        ":endTime, :unpausedDurationSeconds);";
    auto query = QSqlQuery(m_database);
    query.prepare(queryString);

    query.bindValue(":contentId", QString::fromStdString(session.contentId));
    query.bindValue(":slotNumber", session.slotNumber);
    query.bindValue(":startTime", session.startTime);
    query.bindValue(":endTime", session.endTime);
    query.bindValue(":unpausedDurationSeconds",
                    static_cast<uint16_t>(session.unpausedDurationMillis / 1000));

    if (!query.exec()) {
      spdlog::warn("Insert into play_sessions failed: {}",
                   query.lastError().text().toStdString());
      return false;
    }

    session.id = query.lastInsertId().toInt();

    return true;
  }

  std::optional<PlaySession>
  SqliteUserdataDatabase::getLatestPlaySession(const std::string contentId) {
    return std::nullopt;
    // const QString queryString = "SELECT * FROM play_sessions WHERE content_id
    // = "
    //                             ":contentId ORDER BY end_time DESC LIMIT 1;";
    // auto query = QSqlQuery(m_database);
    // query.prepare(queryString);
    //
    // query.bindValue(":contentId", QString::fromStdString(contentId));
    //
    // if (!query.exec()) {
    //   spdlog::warn("Retrieving last play_session failed: {}",
    //                query.lastError().text().toStdString());
    //   return std::nullopt;
    // }
    //
    // if (query.next()) {
    //   PlaySession session;
    //   session.id = query.value("id").toInt();
    //   session.contentId = query.value("content_id").toString().toStdString();
    //   session.slotNumber = query.value("savefile_slot_number").toUInt();
    //   session.startTime = query.value("start_time").toLongLong();
    //   session.endTime = query.value("end_time").toLongLong();
    //   session.unpausedDurationSeconds =
    //       query.value("unpaused_duration_seconds").toUInt();
    //
    //   return {session};
    // }
    //
    // return std::nullopt;
  }

  std::optional<std::string> SqliteUserdataDatabase::getPlatformSettingValue(
    const int platformId, const std::string key) {
    const QString queryString = "SELECT value FROM platform_settings "
        "WHERE platform_id = :platformId AND key = :key LIMIT 1;";
    auto query = QSqlQuery(m_database);
    query.prepare(queryString);

    query.bindValue(":platformId", platformId);
    query.bindValue(":key", QString::fromStdString(key));

    if (!query.exec()) {
      spdlog::warn("Get from platform_settings failed: {}",
                   query.lastError().text().toStdString());
      return std::nullopt;
    }

    if (query.next()) {
      return query.value("value").toString().toStdString();
    }

    return std::nullopt;
  }

  std::map<std::string, std::string> SqliteUserdataDatabase::getAllPlatformSettings(int platformId) {
    const QString queryString = "SELECT key, value FROM platform_settings "
        "WHERE platform_id = :platformId;";
    auto query = QSqlQuery(m_database);
    query.prepare(queryString);

    query.bindValue(":platformId", platformId);

    if (!query.exec()) {
      spdlog::warn("Get all from platform_settings failed: {}",
                   query.lastError().text().toStdString());
      return {};
    }

    std::map<std::string, std::string> settings;
    while (query.next()) {
      settings[query.value("key").toString().toStdString()] = query.value("value").toString().toStdString();
    }

    return settings;
  }

  void SqliteUserdataDatabase::setPlatformSettingValue(const int platformId, const std::string key,
                                                       const std::string value) {
    const QString queryString = "INSERT OR REPLACE INTO platform_settings "
        "(platform_id, key, value) "
        "VALUES (:platformId, :key, :value);";
    auto query = QSqlQuery(m_database);
    query.prepare(queryString);

    query.bindValue(":platformId", platformId);
    query.bindValue(":key", QString::fromStdString(key));
    query.bindValue(":value", QString::fromStdString(value));

    if (!query.exec()) {
      spdlog::warn("Insert into platform_settings failed: {}",
                   query.lastError().text().toStdString());
    }
  }
} // namespace firelight::db

#include "sqlite_userdata_database.hpp"

#include <QSqlError>
#include <QSqlQuery>
#include <spdlog/spdlog.h>

namespace firelight::db {

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
                                 "content_md5 TEXT NOT NULL,"
                                 "slot_number INTEGER NOT NULL,"
                                 "savefile_md5 TEXT NOT NULL,"
                                 "last_modified_at INTEGER NOT NULL,"
                                 "created_at INTEGER NOT NULL, "
                                 "UNIQUE(content_md5, slot_number));");

  if (!createSavefileMetadata.exec()) {
    spdlog::error("Table creation failed: {}",
                  createSavefileMetadata.lastError().text().toStdString());
  }

  QSqlQuery createPlaySessions(m_database);
  createPlaySessions.prepare("CREATE TABLE IF NOT EXISTS play_sessions("
                             "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                             "content_md5 TEXT NOT NULL,"
                             "savefile_slot_number INTEGER NOT NULL,"
                             "start_time INTEGER NOT NULL,"
                             "end_time INTEGER NOT NULL,"
                             "unpaused_duration_seconds INTEGER NOT NULL);");

  if (!createPlaySessions.exec()) {
    spdlog::error("Table creation failed: {}",
                  createPlaySessions.lastError().text().toStdString());
  }
}

SqliteUserdataDatabase::~SqliteUserdataDatabase() {
  m_database.close();
  QSqlDatabase::removeDatabase(m_database.connectionName());
}

std::vector<SavefileMetadata>
SqliteUserdataDatabase::getSavefileMetadataForContent(
    const std::string contentMd5) {
  QSqlQuery query(m_database);
  query.prepare(
      "SELECT * FROM savefile_metadata WHERE content_md5 = :contentMd5;");
  query.bindValue(":contentMd5", QString::fromStdString(contentMd5));

  if (!query.exec()) {
    spdlog::warn("Could not retrieve savefile metadata: {}",
                 query.lastError().text().toStdString());
    return {};
  }

  std::vector<SavefileMetadata> metadataList;
  while (query.next()) {
    SavefileMetadata metadata;
    metadata.id = query.value("id").toUInt();
    metadata.contentMd5 = query.value("content_md5").toString().toStdString();
    metadata.slotNumber = query.value("slot_number").toUInt();
    metadata.savefileMd5 = query.value("savefile_md5").toString().toStdString();
    metadata.lastModifiedAt = query.value("last_modified_at").toLongLong();
    metadata.createdAt = query.value("created_at").toLongLong();
    metadataList.emplace_back(metadata);
  }

  return metadataList;
}

bool SqliteUserdataDatabase::tableExists(const std::string tableName) {
  QSqlQuery query(m_database);
  query.prepare("SELECT 1 FROM " + QString::fromStdString(tableName) +
                " LIMIT 1;");

  return query.exec();
}
std::optional<SavefileMetadata>
SqliteUserdataDatabase::getSavefileMetadata(const std::string contentMd5,
                                            const uint8_t slotNumber) {
  const QString queryString =
      "SELECT * FROM savefile_metadata WHERE content_md5 = :contentMd5 AND "
      "slot_number = :slotNumber LIMIT 1;";
  QSqlQuery query(m_database);
  query.prepare(queryString);
  query.bindValue(":contentMd5", QString::fromStdString(contentMd5));
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
  metadata.contentMd5 = query.value("content_md5").toString().toStdString();
  metadata.slotNumber = query.value("slot_number").toUInt();
  metadata.savefileMd5 = query.value("savefile_md5").toString().toStdString();
  metadata.lastModifiedAt = query.value("last_modified_at").toLongLong();
  metadata.createdAt = query.value("created_at").toLongLong();

  return metadata;
}

void SqliteUserdataDatabase::createPlaySession(PlaySession session) {
  const QString queryString =
      "INSERT INTO play_sessions ("
      "content_md5, "
      "start_time, "
      "end_time, "
      "unpaused_duration_seconds) "
      "VALUES (:contentMd5, :startTime, :endTime, :unpausedDurationSeconds);";

  auto db = QSqlDatabase::database("userdata");
  if (!db.isValid()) {
    // TODO: Give each thread its own connection name?
    db = QSqlDatabase::addDatabase("QSQLITE", "userdata");
    db.setDatabaseName(QString::fromStdString(m_database_path.string()));
    db.open();
  }

  auto query = QSqlQuery(db);
  query.prepare(queryString);

  query.bindValue(":contentMd5", QString::fromStdString(session.contentMd5));
  query.bindValue(":startTime", session.startTime);
  query.bindValue(":endTime", session.endTime);
  query.bindValue(":unpausedDurationSeconds", session.unpausedDurationSeconds);

  if (!query.exec()) {
    spdlog::warn("Insert into play_sessions failed: {}",
                 query.lastError().text().toStdString());
  }
}

bool SqliteUserdataDatabase::updateSavefileMetadata(SavefileMetadata metadata) {
  QSqlQuery query(m_database);
  query.prepare("UPDATE savefile_metadata SET savefile_md5 = :savefileMd5, "
                "last_modified_at = :lastModifiedAt WHERE id = :id;");
  query.bindValue(":savefileMd5", QString::fromStdString(metadata.savefileMd5));
  query.bindValue(":lastModifiedAt", metadata.lastModifiedAt);
  query.bindValue(":id", metadata.id);

  if (!query.exec()) {
    spdlog::error("Update failed: {}", query.lastError().text().toStdString());
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

  const QString queryString = "INSERT INTO savefile_metadata (content_md5, "
                              "slot_number, savefile_md5, last_modified_at, "
                              "created_at) VALUES (:contentMd5, :slotNumber, "
                              ":savefileMd5, :lastModifiedAt, :createdAt);";

  QSqlQuery query(m_database);
  query.prepare(queryString);
  query.bindValue(":contentMd5", QString::fromStdString(metadata.contentMd5));
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

} // namespace firelight::db

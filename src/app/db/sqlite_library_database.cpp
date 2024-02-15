//
// Created by alexs on 12/29/2023.
//

#include "sqlite_library_database.hpp"

#include <QSqlError>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <spdlog/spdlog.h>
#include <utility>

const char *create_query = "CREATE TABLE IF NOT EXISTS library("
                           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                           "display_name NVARCHAR(999) NOT NULL,"
                           "type INTEGER NOT NULL,"
                           "verified INTEGER,"
                           "md5 NVARCHAR(999) NOT NULL UNIQUE,"
                           "platform INTEGER NOT NULL,"
                           "game INTEGER,"
                           "rom INTEGER,"
                           "parent_entry INTEGER,"
                           "romhack INTEGER,"
                           "romhack_release INTEGER,"
                           "source_directory NVARCHAR(999) NOT NULL,"
                           "content_path NVARCHAR(999) NOT NULL);";
// "CREATE UNIQUE INDEX IF NOT EXISTS idx_md5 ON library (md5);";

void SqliteLibraryDatabase::updateEntryContentPath(
    const int entryId, const std::string sourceDirectory,
    const std::string contentPath) {
  const QString queryString =
      "UPDATE library "
      "SET source_directory = :source_directory, content_path = :content_path "
      "WHERE id = :id;";

  auto db = QSqlDatabase::database("library");
  if (!db.isValid()) {
    // TODO: Give each thread its own connection name?
    db = QSqlDatabase::addDatabase("QSQLITE", "library");
    db.setDatabaseName(QString::fromStdString(database_file_path_.string()));
    db.open();
  }

  auto query = QSqlQuery(db);
  query.prepare(queryString);

  query.bindValue(":id", entryId);
  query.bindValue(":source_directory", QString::fromStdString(sourceDirectory));
  query.bindValue(":content_path", QString::fromStdString(contentPath));

  if (!query.exec()) {
    spdlog::warn("Update failed: {}", query.lastError().text().toStdString());
  }
}

SqliteLibraryDatabase::SqliteLibraryDatabase(std::filesystem::path db_file_path)
    : database_file_path_(std::move(db_file_path)) {}

bool SqliteLibraryDatabase::initialize() {
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "library");
  db.setDatabaseName(QString::fromStdString(database_file_path_.string()));
  if (!db.open()) {
    printf("couldn't connect for some reason\n");
  }

  QSqlQuery create(db);
  create.prepare(create_query);
  if (!create.exec()) {
    spdlog::error("Table creation failed: {}",
                  create.lastError().text().toStdString());
  }

  return true;
}

void SqliteLibraryDatabase::addOrRenameEntry(const LibEntry entry) {
  insert_entry_into_db(entry);
}

std::vector<LibEntry> SqliteLibraryDatabase::getAllEntries() {
  return getMatching({});
}

void SqliteLibraryDatabase::match_md5s(std::string source_directory,
                                       std::vector<std::string> md5s) {
  // TODO: Select all from DB with matching source
  // LibraryDatabase::match_md5s(source_directory, md5s);
}
std::vector<LibEntry> SqliteLibraryDatabase::getMatching(Filter filter) {
  std::vector<LibEntry> result;

  QString queryString = "SELECT * FROM library";

  auto db = QSqlDatabase::database("library");
  if (!db.isValid()) {
    // TODO: Give each thread its own connection name?
    db = QSqlDatabase::addDatabase("QSQLITE", "library");
    db.setDatabaseName(QString::fromStdString(database_file_path_.string()));
    db.open();
  }

  QString whereClause;
  bool needAND = false;

  if (filter.id != -1) {
    whereClause += " WHERE id = :id";
    needAND = true;
  }

  if (!filter.display_name.empty()) {
    if (needAND) {
      whereClause += " AND ";
    } else {
      whereClause += " WHERE ";
      needAND = true;
    }
    whereClause += "display_name = :display_name";
  }

  if (!filter.md5.empty()) {
    if (needAND) {
      whereClause += " AND ";
    } else {
      whereClause += " WHERE ";
      needAND = true;
    }
    whereClause += "md5 = :md5";
  }

  if (!filter.content_path.empty()) {
    if (needAND) {
      whereClause += " AND ";
    } else {
      whereClause += " WHERE ";
      needAND = true;
    }
    whereClause += "content_path = :content_path";
  }

  if (filter.rom != -1) {
    if (needAND) {
      whereClause += " AND ";
    } else {
      whereClause += " WHERE ";
      needAND = true;
    }
    whereClause += "rom = :rom";
  }

  queryString += whereClause;
  QSqlQuery query(db);
  query.prepare(queryString);

  if (filter.id != -1) {
    query.bindValue(":id", filter.id);
  }
  if (!filter.display_name.empty()) {
    query.bindValue(":display_name",
                    QString::fromStdString(filter.display_name));
  }
  if (!filter.md5.empty()) {
    query.bindValue(":md5", QString::fromStdString(filter.md5));
  }
  if (!filter.content_path.empty()) {
    query.bindValue(":content_path",
                    QString::fromStdString(filter.content_path));
  }
  if (filter.rom != -1) {
    query.bindValue(":rom", filter.rom);
  }

  if (!query.exec()) {
    spdlog::error("ruh roh raggy: {}", query.lastError().text().toStdString());
  }

  while (query.next()) {
    LibEntry nextEntry;
    nextEntry.id = query.value(0).toInt();
    nextEntry.display_name = query.value(1).toString().toStdString();
    nextEntry.type = static_cast<EntryType>(query.value(2).toInt());
    nextEntry.verified = query.value(3).toBool();
    nextEntry.md5 = query.value(4).toString().toStdString();
    nextEntry.platform = query.value(5).toInt();
    nextEntry.game = query.value(6).toInt();
    nextEntry.rom = query.value(7).toInt();
    nextEntry.parent_entry = query.value(8).toInt();
    nextEntry.romhack = query.value(9).toInt();
    nextEntry.romhack_release = query.value(10).toInt();
    nextEntry.source_directory = query.value(11).toString().toStdString();
    nextEntry.content_path = query.value(12).toString().toStdString();

    result.emplace_back(nextEntry);
  }

  return result;
}

void SqliteLibraryDatabase::insert_entry_into_db(LibEntry entry) const {
  QString queryString =
      "INSERT OR IGNORE INTO library ("
      "display_name, "
      "type, "
      "verified, "
      "md5, "
      "platform,"
      "game, "
      "rom, "
      "parent_entry, "
      "romhack, "
      "romhack_release, "
      "source_directory, "
      "content_path) "
      "VALUES (:display_name, :type, :verified, :md5, :platform, :game, "
      ":rom, :parent_entry, :romhack, :romhack_release, :source_directory, "
      ":content_path);";

  auto db = QSqlDatabase::database("library");
  if (!db.isValid()) {
    // TODO: Give each thread its own connection name?
    db = QSqlDatabase::addDatabase("QSQLITE", "library");
    db.setDatabaseName(QString::fromStdString(database_file_path_.string()));
    db.open();
  }

  QSqlQuery query = QSqlQuery(db);
  query.prepare(queryString);

  query.bindValue(":display_name", QString::fromStdString(entry.display_name));
  query.bindValue(":type", static_cast<int>(entry.type));
  query.bindValue(":verified", entry.verified);
  query.bindValue(":md5", QString::fromStdString(entry.md5));
  query.bindValue(":platform", entry.platform);
  query.bindValue(":game", entry.game);
  query.bindValue(":rom", entry.rom);
  query.bindValue(":parent_entry", entry.parent_entry);
  query.bindValue(":romhack", entry.romhack);
  query.bindValue(":romhack_release", entry.romhack_release);
  query.bindValue(":source_directory",
                  QString::fromStdString(entry.source_directory));
  query.bindValue(":content_path", QString::fromStdString(entry.content_path));

  if (!query.exec()) {
    spdlog::warn("Insert into library failed: {}",
                 query.lastError().text().toStdString());
    return;
  }

  if (query.numRowsAffected() > 0) {
    entry.id = query.lastInsertId().toInt();
  } else {
    // The row already exists, retrieve its ID
    QSqlQuery selectQuery(db);
    selectQuery.prepare("SELECT id FROM library WHERE md5 = :md5");
    selectQuery.bindValue(":md5", QString::fromStdString(entry.md5));

    if (selectQuery.exec() && selectQuery.next()) {
      entry.id = selectQuery.value(0).toInt();
    } else {
      spdlog::warn("Retrieving existing library row failed: {}",
                   selectQuery.lastError().text().toStdString());
    }
  }
}
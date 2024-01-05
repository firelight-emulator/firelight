//
// Created by alexs on 12/29/2023.
//

#include "sqlite_library_database.hpp"

#include <QSqlError>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <spdlog/spdlog.h>
#include <utility>

const char *create_query =
    "CREATE TABLE IF NOT EXISTS library("
    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "display_name NVARCHAR(999) NOT NULL,"
    "verified INTEGER,"
    "md5 NVARCHAR(999) NOT NULL UNIQUE,"
    "platform INTEGER NOT NULL,"
    "game INTEGER,"
    "rom INTEGER,"
    "romhack INTEGER,"
    "source_directory NVARCHAR(999) NOT NULL,"
    "content_path NVARCHAR(999) NOT NULL);"
    "CREATE UNIQUE INDEX IF NOT EXISTS idx_md5 ON library (md5);";

SqliteLibraryDatabase::SqliteLibraryDatabase(std::filesystem::path db_file_path)
    : database_file_path_(std::move(db_file_path)) {}

bool SqliteLibraryDatabase::initialize() {
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "library");
  db.setDatabaseName(QString::fromStdString(database_file_path_.string()));
  if (!db.open()) {
    printf("couldn't connect for some reason\n");
  }

  QSqlQuery create(create_query, db);
  create.exec();

  // qdatabase_.setDatabaseName(
  //     QString::fromStdString(database_file_path_.string()));
  // if (!qdatabase_.open()) {
  //   printf("couldn't connect for some reason\n");
  // }

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

  return true;
}

void SqliteLibraryDatabase::addOrRenameEntry(const LibEntry entry) {
  insert_entry_into_db(entry);
}

std::vector<LibEntry> SqliteLibraryDatabase::getAllEntries() {
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
    results.emplace_back(entry_from_stmt(stmt));
    // TODO: Handle error
  }

  return results;
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

  if (!query.exec()) {
    spdlog::error("ruh roh raggy\n");
  }

  while (query.next()) {
    LibEntry nextEntry;
    nextEntry.id = query.value(0).toInt();
    nextEntry.display_name = query.value(1).toString().toStdString();
    nextEntry.verified = query.value(2).toBool();
    nextEntry.md5 = query.value(3).toString().toStdString();
    nextEntry.platform = query.value(4).toBool();
    nextEntry.game = query.value(5).toBool();
    nextEntry.rom = query.value(6).toBool();
    nextEntry.romhack = query.value(7).toBool();
    nextEntry.source_directory = query.value(8).toString().toStdString();
    nextEntry.content_path = query.value(9).toString().toStdString();

    result.emplace_back(nextEntry);
  }

  return result;
}

LibEntry SqliteLibraryDatabase::entry_from_stmt(sqlite3_stmt *stmt) {
  LibEntry entry;
  entry.id = sqlite3_column_int(stmt, 0);
  entry.display_name = std::string(reinterpret_cast<char *>(
      const_cast<unsigned char *>(sqlite3_column_text(stmt, 1))));
  entry.verified = sqlite3_column_int(stmt, 2);
  entry.md5 = std::string(reinterpret_cast<char *>(
      const_cast<unsigned char *>(sqlite3_column_text(stmt, 3))));
  entry.platform = sqlite3_column_int(stmt, 4);
  entry.game = sqlite3_column_int(stmt, 5);
  entry.rom = sqlite3_column_int(stmt, 6);
  entry.romhack = sqlite3_column_int(stmt, 7);
  entry.source_directory = std::string(reinterpret_cast<char *>(
      const_cast<unsigned char *>(sqlite3_column_text(stmt, 8))));
  entry.content_path = std::string(reinterpret_cast<char *>(
      const_cast<unsigned char *>(sqlite3_column_text(stmt, 9))));

  return entry;
}

void SqliteLibraryDatabase::insert_entry_into_db(LibEntry entry) const {
  // const auto db = getOrCreateDbConnection();
  sqlite3_stmt *stmt = nullptr;
  const char *query = "INSERT OR IGNORE INTO library ("
                      "display_name, "
                      "verified, "
                      "md5, "
                      "platform,"
                      "game, "
                      "rom, "
                      "romhack, "
                      "source_directory, "
                      "content_path) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";

  if (sqlite3_prepare_v2(database_, query, -1, &stmt, nullptr) != SQLITE_OK) {
    printf("prepare didn't work: %s\n", sqlite3_errmsg(database_));
    // Handle error (use sqlite3_errmsg(db) to get the error message)
    sqlite3_finalize(
        stmt); // Always finalize a prepared statement to avoid resource leaks
    return;
  }

  sqlite3_bind_text(stmt, 1, entry.display_name.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_int(stmt, 2, entry.verified ? 1 : 0);
  sqlite3_bind_text(stmt, 3, entry.md5.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_int(stmt, 4, entry.platform);
  sqlite3_bind_int(stmt, 5, entry.game);
  sqlite3_bind_int(stmt, 6, entry.rom);
  sqlite3_bind_int(stmt, 7, entry.romhack);
  sqlite3_bind_text(stmt, 8, entry.source_directory.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 9, entry.content_path.c_str(), -1, SQLITE_STATIC);

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
#include "sqlite_library_database.hpp"

#include <QSqlError>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <spdlog/spdlog.h>

namespace firelight::db {
SqliteLibraryDatabase::SqliteLibraryDatabase(
    const std::filesystem::path &db_file_path) {
  m_database = QSqlDatabase::addDatabase("QSQLITE", "library");
  m_database.setDatabaseName(QString::fromStdString(db_file_path.string()));
  m_database.open();

  QSqlQuery createLibraryEntries(m_database);
  createLibraryEntries.prepare("CREATE TABLE IF NOT EXISTS library_entries("
                               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                               "display_name TEXT NOT NULL,"
                               "content_md5 TEXT UNIQUE NOT NULL,"
                               "platform_id INTEGER NOT NULL,"
                               "type INTEGER NOT NULL,"
                               "source_directory TEXT NOT NULL,"
                               "content_path TEXT NOT NULL);");

  if (!createLibraryEntries.exec()) {
    spdlog::error("Table creation failed: {}",
                  createLibraryEntries.lastError().text().toStdString());
  }

  QSqlQuery createPlaylists(m_database);
  createPlaylists.prepare("CREATE TABLE IF NOT EXISTS playlists("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                          "display_name TEXT UNIQUE NOT NULL);");

  if (!createPlaylists.exec()) {
    spdlog::error("Table creation failed: {}",
                  createPlaylists.lastError().text().toStdString());
  }

  QSqlQuery createPlaylistEntries(m_database);
  createPlaylistEntries.prepare(
      "CREATE TABLE IF NOT EXISTS playlist_entries("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "playlist_id INTEGER NOT NULL,"
      "library_entry_id INTEGER NOT NULL,"
      "FOREIGN KEY(playlist_id) REFERENCES playlists(id),"
      "FOREIGN KEY(library_entry_id) REFERENCES library_entries(id),"
      "UNIQUE(playlist_id, library_entry_id));");

  if (!createPlaylistEntries.exec()) {
    spdlog::error("Table creation failed: {}",
                  createPlaylistEntries.lastError().text().toStdString());
  }
}

SqliteLibraryDatabase::~SqliteLibraryDatabase() {
  m_database.close();
  QSqlDatabase::removeDatabase(m_database.connectionName());
}

bool SqliteLibraryDatabase::tableExists(const std::string &tableName) {
  QSqlQuery query(m_database);
  query.prepare("SELECT 1 FROM " + QString::fromStdString(tableName) +
                " LIMIT 1;");

  return query.exec();
}

bool SqliteLibraryDatabase::createPlaylist(Playlist &playlist) {
  if (!m_database.open()) {
    spdlog::error("Couldn't open database: {}",
                  m_database.lastError().text().toStdString());
    return false;
  }

  const QString queryString =
      "INSERT INTO playlists (display_name) VALUES (:name);";
  QSqlQuery query(m_database);
  query.prepare(queryString);
  query.bindValue(":name", QString::fromStdString(playlist.displayName));

  if (!query.exec()) {
    query.finish();
    return false;
  }

  playlist.id = query.lastInsertId().toInt();

  query.finish();
  return true;
}

bool SqliteLibraryDatabase::addEntryToPlaylist(const int playlistId,
                                               const int entryId) {
  if (!m_database.open()) {
    spdlog::error("Couldn't open database: {}",
                  m_database.lastError().text().toStdString());
    return false;
  }

  QSqlQuery query(m_database);
  query.prepare(
      "INSERT INTO playlist_entries (playlist_id, library_entry_id) VALUES "
      "(:playlistId, :libraryEntryId);");
  query.bindValue(":playlistId", playlistId);
  query.bindValue(":libraryEntryId", entryId);

  query.finish();
  return query.exec();
}

void SqliteLibraryDatabase::updateEntryContentPath(
    const int entryId, const std::string sourceDirectory,
    const std::string contentPath) {
  const QString queryString =
      "UPDATE library_entries "
      "SET source_directory = :source_directory, content_path = :content_path "
      "WHERE id = :id;";

  auto query = QSqlQuery(m_database);
  query.prepare(queryString);

  query.bindValue(":id", entryId);
  query.bindValue(":source_directory", QString::fromStdString(sourceDirectory));
  query.bindValue(":content_path", QString::fromStdString(contentPath));

  if (!query.exec()) {
    spdlog::warn("Update failed: {}", query.lastError().text().toStdString());
  }
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
  return result;

  QString queryString = "SELECT * FROM library_entries";

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
  QSqlQuery query(m_database);
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

bool SqliteLibraryDatabase::createLibraryEntry(LibraryEntry &entry) {
  if (!m_database.open()) {
    spdlog::error("Couldn't open database: {}",
                  m_database.lastError().text().toStdString());
    return false;
  }

  const QString queryString =
      "INSERT INTO library_entries (display_name, content_md5, platform_id, "
      "type, source_directory, content_path) VALUES (:displayName, "
      ":contentMd5, :platformId, :type, :sourceDirectory, :contentPath);";
  QSqlQuery query(m_database);
  query.prepare(queryString);
  query.bindValue(":displayName", QString::fromStdString(entry.displayName));
  query.bindValue(":contentMd5", QString::fromStdString(entry.contentMd5));
  query.bindValue(":platformId", entry.platformId);
  query.bindValue(":type", static_cast<int>(entry.type));
  query.bindValue(":sourceDirectory",
                  QString::fromStdString(entry.sourceDirectory));
  query.bindValue(":contentPath", QString::fromStdString(entry.contentPath));

  if (!query.exec()) {
    query.finish();
    return false;
  }

  entry.id = query.lastInsertId().toInt();

  query.finish();
  return true;
}

std::optional<LibraryEntry>
SqliteLibraryDatabase::getLibraryEntry(const int entryId) {
  QSqlQuery q(m_database);
  q.prepare("SELECT * FROM library_entries WHERE id = :id");
  q.bindValue(":id", entryId);

  if (!q.exec()) {
    spdlog::error("Failed to get library entry: {}",
                  q.lastError().text().toStdString());
    return std::nullopt;
  }

  if (!q.next()) {
    return std::nullopt;
  }

  LibraryEntry entry;
  entry.id = q.value(0).toInt();
  entry.displayName = q.value(1).toString().toStdString();
  entry.contentMd5 = q.value(2).toString().toStdString();
  entry.platformId = q.value(3).toInt();
  entry.type = static_cast<LibraryEntry::EntryType>(q.value(4).toInt());
  entry.sourceDirectory = q.value(5).toString().toStdString();
  entry.contentPath = q.value(6).toString().toStdString();

  return entry;
}

bool SqliteLibraryDatabase::deleteLibraryEntry(int entryId) {
  QSqlQuery query(m_database);
  query.prepare("DELETE FROM library_entries WHERE id = :id");
  query.bindValue(":id", entryId);

  if (!query.exec()) {
    spdlog::error("Failed to delete library entry: {}",
                  query.lastError().text().toStdString());
    return false;
  }

  return true;
}

std::vector<LibraryEntry> SqliteLibraryDatabase::getAllLibraryEntries() {
  QSqlQuery q(m_database);
  q.prepare("SELECT * FROM library_entries");

  if (!q.exec()) {
    spdlog::error("Failed to get library entries: {}",
                  q.lastError().text().toStdString());
  }

  std::vector<LibraryEntry> entries;
  while (q.next()) {
    LibraryEntry entry;
    entry.id = q.value(0).toInt();
    entry.displayName = q.value(1).toString().toStdString();
    entry.contentMd5 = q.value(2).toString().toStdString();
    entry.platformId = q.value(3).toInt();
    entry.type = static_cast<LibraryEntry::EntryType>(q.value(4).toInt());
    entry.sourceDirectory = q.value(5).toString().toStdString();
    entry.contentPath = q.value(6).toString().toStdString();

    entries.emplace_back(entry);
  }

  return entries;
}

std::vector<Playlist> SqliteLibraryDatabase::getAllPlaylists() {
  QSqlQuery q(m_database);
  q.prepare("SELECT * FROM playlists");

  if (!q.exec()) {
    spdlog::error("Failed to get playlists: {}",
                  q.lastError().text().toStdString());
  }

  std::vector<Playlist> playlists;
  while (q.next()) {
    Playlist playlist;
    playlist.id = q.value(0).toInt();
    playlist.displayName = q.value(1).toString().toStdString();

    playlists.emplace_back(playlist);
  }

  return playlists;
}

bool SqliteLibraryDatabase::deletePlaylist(const int playlistId) {
  QSqlQuery query(m_database);
  query.prepare("DELETE FROM playlists WHERE id = :id");
  query.bindValue(":id", playlistId);

  if (!query.exec()) {
    spdlog::error("Failed to delete playlist: {}",
                  query.lastError().text().toStdString());
    return false;
  }

  return true;
}

bool SqliteLibraryDatabase::renamePlaylist(const int playlistId,
                                           const std::string newName) {
  QSqlQuery checkQuery(m_database);
  checkQuery.prepare("SELECT 1 FROM playlists WHERE id = :id");
  checkQuery.bindValue(":id", playlistId);
  if (!checkQuery.exec()) {
    spdlog::error("Error retrieving playlists: {}",
                  checkQuery.lastError().text().toStdString());
    return false;
  }

  int numRows = 0;
  while (checkQuery.next()) {
    numRows++;
  }

  if (numRows == 0) {
    spdlog::warn("Playlist with ID {} does not exist", playlistId);
    return false;
  }

  QSqlQuery query(m_database);
  query.prepare("UPDATE playlists SET display_name = :name WHERE id = :id");
  query.bindValue(":name", QString::fromStdString(newName));
  query.bindValue(":id", playlistId);

  if (!query.exec()) {
    spdlog::error("Failed to rename playlist: {}",
                  query.lastError().text().toStdString());
    return false;
  }

  return true;
}

void SqliteLibraryDatabase::insert_entry_into_db(LibEntry entry) const {
  return;
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

  QSqlQuery query = QSqlQuery(m_database);
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
    QSqlQuery selectQuery(m_database);
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

} // namespace firelight::db

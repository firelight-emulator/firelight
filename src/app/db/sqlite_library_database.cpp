#include "sqlite_library_database.hpp"

#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QThread>
#include <spdlog/spdlog.h>

constexpr auto DATABASE_PREFIX = "library_";

namespace firelight::db {
SqliteLibraryDatabase::SqliteLibraryDatabase(
    const std::filesystem::path &db_file_path)
    : m_dbFilePath(db_file_path) {

  const auto db = getDatabase();

  QSqlQuery createLibraryEntries(db);
  createLibraryEntries.prepare("CREATE TABLE IF NOT EXISTS library_entries("
                               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                               "display_name TEXT NOT NULL,"
                               "content_md5 TEXT UNIQUE NOT NULL,"
                               "platform_id INTEGER NOT NULL,"
                               "active_save_slot INTEGER DEFAULT 1,"
                               "type INTEGER NOT NULL,"
                               "source_directory TEXT NOT NULL,"
                               "content_path TEXT NOT NULL,"
                               "created_at INTEGER NOT NULL);");

  if (!createLibraryEntries.exec()) {
    spdlog::error("Table creation failed: {}",
                  createLibraryEntries.lastError().text().toStdString());
  }

  QSqlQuery createPlaylists(db);
  createPlaylists.prepare("CREATE TABLE IF NOT EXISTS playlists("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                          "display_name TEXT UNIQUE NOT NULL,"
                          "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);");

  if (!createPlaylists.exec()) {
    spdlog::error("Table creation failed: {}",
                  createPlaylists.lastError().text().toStdString());
  }

  QSqlQuery createPlaylistEntries(db);
  createPlaylistEntries.prepare(
      "CREATE TABLE IF NOT EXISTS playlist_entries("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "playlist_id INTEGER NOT NULL,"
      "library_entry_id INTEGER NOT NULL,"
      "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
      "FOREIGN KEY(playlist_id) REFERENCES playlists(id),"
      "FOREIGN KEY(library_entry_id) REFERENCES library_entries(id),"
      "UNIQUE(playlist_id, library_entry_id));");

  if (!createPlaylistEntries.exec()) {
    spdlog::error("Table creation failed: {}",
                  createPlaylistEntries.lastError().text().toStdString());
  }
}

SqliteLibraryDatabase::~SqliteLibraryDatabase() {
  for (const auto &name : QSqlDatabase::connectionNames()) {
    if (name.startsWith(DATABASE_PREFIX)) {
      QSqlDatabase::removeDatabase(name);
    }
  }
}

bool SqliteLibraryDatabase::tableExists(const std::string &tableName) {
  QSqlQuery query(getDatabase());
  query.prepare("SELECT 1 FROM " + QString::fromStdString(tableName) +
                " LIMIT 1;");

  return query.exec();
}

bool SqliteLibraryDatabase::createPlaylist(Playlist &playlist) {
  const QString queryString =
      "INSERT INTO playlists (display_name) VALUES (:name);";
  QSqlQuery query(getDatabase());
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
  QSqlQuery query(getDatabase());
  query.prepare(
      "INSERT INTO playlist_entries (playlist_id, library_entry_id) VALUES "
      "(:playlistId, :libraryEntryId);");
  query.bindValue(":playlistId", playlistId);
  query.bindValue(":libraryEntryId", entryId);

  query.finish();
  return query.exec();
}

bool SqliteLibraryDatabase::createLibraryEntry(LibraryEntry &entry) {
  const QString queryString =
      "INSERT INTO library_entries (display_name, content_md5, platform_id, "
      "type, source_directory, content_path, created_at) VALUES (:displayName, "
      ":contentMd5, :platformId, :type, :sourceDirectory, :contentPath, "
      ":createdAt);";
  QSqlQuery query(getDatabase());
  query.prepare(queryString);
  query.bindValue(":displayName", QString::fromStdString(entry.displayName));
  query.bindValue(":contentMd5", QString::fromStdString(entry.contentMd5));
  query.bindValue(":platformId", entry.platformId);
  query.bindValue(":type", static_cast<int>(entry.type));
  query.bindValue(":sourceDirectory",
                  QString::fromStdString(entry.sourceDirectory));
  query.bindValue(":contentPath", QString::fromStdString(entry.contentPath));
  query.bindValue(":createdAt",
                  std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::system_clock::now().time_since_epoch())
                      .count());

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
  QSqlQuery q(getDatabase());
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

  return createLibraryEntryFromQuery(q);
}

bool SqliteLibraryDatabase::deleteLibraryEntry(int entryId) {
  QSqlQuery query(getDatabase());
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
  QSqlQuery q(getDatabase());
  q.prepare("SELECT * FROM library_entries");

  if (!q.exec()) {
    spdlog::error("Failed to get library entries: {}",
                  q.lastError().text().toStdString());
  }

  std::vector<LibraryEntry> entries;
  while (q.next()) {
    entries.emplace_back(createLibraryEntryFromQuery(q));
  }

  return entries;
}

std::vector<LibraryEntry>
SqliteLibraryDatabase::getMatchingLibraryEntries(const LibraryEntry &entry) {
  QString queryString = "SELECT * FROM library_entries";

  QString whereClause;
  bool needAND = false;

  if (entry.id != -1) {
    whereClause += " WHERE id = :id";
    needAND = true;
  }

  if (!entry.displayName.empty()) {
    if (needAND) {
      whereClause += " AND ";
    } else {
      whereClause += " WHERE ";
      needAND = true;
    }
    whereClause += "display_name = :displayName";
  }

  if (!entry.contentMd5.empty()) {
    if (needAND) {
      whereClause += " AND ";
    } else {
      whereClause += " WHERE ";
      needAND = true;
    }
    whereClause += "content_md5 = :contentMd5";
  }

  if (!entry.contentPath.empty()) {
    if (needAND) {
      whereClause += " AND ";
    } else {
      whereClause += " WHERE ";
      needAND = true;
    }
    whereClause += "content_path = :contentPath";
  }

  queryString += whereClause;
  QSqlQuery query(getDatabase());
  query.prepare(queryString);

  if (entry.id != -1) {
    query.bindValue(":id", entry.id);
  }
  if (!entry.displayName.empty()) {
    query.bindValue(":displayName", QString::fromStdString(entry.displayName));
  }
  if (!entry.contentMd5.empty()) {
    query.bindValue(":contentMd5", QString::fromStdString(entry.contentMd5));
  }
  if (!entry.contentPath.empty()) {
    query.bindValue(":contentPath", QString::fromStdString(entry.contentPath));
  }

  if (!query.exec()) {
    spdlog::error("ruh roh raggy: {}", query.lastError().text().toStdString());
  }

  std::vector<LibraryEntry> entries;

  while (query.next()) {
    entries.emplace_back(createLibraryEntryFromQuery(query));
  }

  return entries;
}

std::vector<Playlist> SqliteLibraryDatabase::getAllPlaylists() {
  QSqlQuery q(getDatabase());
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

std::vector<Playlist>
SqliteLibraryDatabase::getPlaylistsForEntry(const int entryId) {
  QSqlQuery query(getDatabase());
  query.prepare(
      "SELECT playlists.* FROM playlists "
      "JOIN playlist_entries ON playlists.id = playlist_entries.playlist_id "
      "WHERE playlist_entries.library_entry_id = :entryId");
  query.bindValue(":entryId", entryId);

  if (!query.exec()) {
    spdlog::error("Failed to retrieve playlists: {}",
                  query.lastError().text().toStdString());
    return {};
  }

  std::vector<Playlist> playlists;
  while (query.next()) {
    Playlist playlist;
    playlist.id = query.value(0).toInt();
    playlist.displayName = query.value(1).toString().toStdString();

    playlists.emplace_back(playlist);
  }

  return playlists;
}

bool SqliteLibraryDatabase::deletePlaylist(const int playlistId) {
  QSqlQuery query(getDatabase());
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
  QSqlQuery checkQuery(getDatabase());
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

  QSqlQuery query(getDatabase());
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

LibraryEntry
SqliteLibraryDatabase::createLibraryEntryFromQuery(const QSqlQuery &query) {
  LibraryEntry entry;
  entry.id = query.value(0).toInt();
  entry.displayName = query.value(1).toString().toStdString();
  entry.contentMd5 = query.value(2).toString().toStdString();
  entry.platformId = query.value(3).toInt();
  entry.activeSaveSlot = query.value(4).toInt();
  entry.type = static_cast<LibraryEntry::EntryType>(query.value(5).toInt());
  entry.sourceDirectory = query.value(6).toString().toStdString();
  entry.contentPath = query.value(7).toString().toStdString();
  entry.createdAt = query.value(8).toLongLong();

  return entry;
}

QSqlDatabase SqliteLibraryDatabase::getDatabase() const {
  const auto name =
      DATABASE_PREFIX +
      QString::number(reinterpret_cast<quint64>(QThread::currentThread()), 16);
  if (QSqlDatabase::contains(name)) {
    return QSqlDatabase::database(name);
  }

  spdlog::debug("Database connection with name {} does not exist; creating",
                name.toStdString());
  auto db = QSqlDatabase::addDatabase("QSQLITE", name);
  db.setDatabaseName(QString::fromStdString(m_dbFilePath.string()));
  db.open();
  return db;
}

} // namespace firelight::db

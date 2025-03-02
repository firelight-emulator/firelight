#include "sqlite_library_database.hpp"

#include <QDateTime>
#include <QJsonObject>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QThread>
#include <QUrl>
#include <spdlog/spdlog.h>
#include <utility>

constexpr auto DATABASE_PREFIX = "library_";

namespace firelight::db {
  SqliteLibraryDatabase::SqliteLibraryDatabase(QString db_file_path)
    : m_dbFilePath(std::move(db_file_path)) {
    const auto db = getDatabase();

    QSqlQuery createLibraryEntries(db);
    createLibraryEntries.prepare("CREATE TABLE IF NOT EXISTS library_entries("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "display_name TEXT NOT NULL,"
      "content_id TEXT UNIQUE NOT NULL,"
      "platform_id INTEGER NOT NULL,"
      "parent_entry_id INTEGER DEFAULT -1,"
      "game_id INTEGER DEFAULT -1,"
      "mod_id INTEGER DEFAULT -1,"
      "active_save_slot INTEGER DEFAULT 1,"
      "type INTEGER NOT NULL,"
      "file_md5 TEXT UNIQUE NOT NULL,"
      "file_crc32 TEXT UNIQUE NOT NULL,"
      "source_directory TEXT NOT NULL,"
      "content_path TEXT NOT NULL,"
      "created_at INTEGER NOT NULL);");

    if (!createLibraryEntries.exec()) {
      spdlog::error("Table creation failed: {}",
                    createLibraryEntries.lastError().text().toStdString());
    }

    QSqlQuery createContentDirectories(db);
    createContentDirectories.prepare(
      "CREATE TABLE IF NOT EXISTS content_directories("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "path TEXT UNIQUE NOT NULL,"
      "num_game_files INTEGER DEFAULT 0,"
      "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);");

    if (!createContentDirectories.exec()) {
      spdlog::error("Table creation failed: {}",
                    createContentDirectories.lastError().text().toStdString());
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
    for (const auto &name: QSqlDatabase::connectionNames()) {
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

  QVariant SqliteLibraryDatabase::getLibraryEntryJson(int entryId) {
    auto entry = getLibraryEntry(entryId);
    if (!entry.has_value()) {
      return QVariant{};
    }

    QJsonObject jsonObj;
    jsonObj["id"] = entry->id;
    jsonObj["display_name"] = QString::fromStdString(entry->displayName);
    jsonObj["platform_name"] = "Lol cool";
    jsonObj["game_id"] = entry->gameId;
    jsonObj["parent_entry_id"] = entry->parentEntryId;
    jsonObj["is_patch"] = entry->type == LibraryEntry::EntryType::PATCH;
    jsonObj["content_path"] = QString::fromStdString(entry->contentPath);
    jsonObj["active_save_slot"] = static_cast<int>(entry->activeSaveSlot);
    jsonObj["created_at"] = QJsonValue::fromVariant(entry->createdAt);

    return QVariant::fromValue(jsonObj);
  }

  bool SqliteLibraryDatabase::createLibraryEntry(LibraryEntry &entry) {
    const QString queryString =
        "INSERT INTO library_entries (display_name, content_id, "
        "platform_id, "
        "parent_entry_id, game_id, mod_id, type, file_md5, file_crc32, "
        "source_directory, "
        "content_path, "
        "created_at) VALUES"
        "(:displayName, :contentId, :platformId, :parentEntryId, :gameId, "
        ":modId, :type, :fileMd5, :fileCrc32, :sourceDirectory, :contentPath, "
        ":createdAt);";
    QSqlQuery query(getDatabase());
    query.prepare(queryString);
    query.bindValue(":displayName", QString::fromStdString(entry.displayName));
    query.bindValue(":contentId", QString::fromStdString(entry.contentId));
    query.bindValue(":platformId", entry.platformId);
    query.bindValue(":parentEntryId", entry.parentEntryId);
    query.bindValue(":gameId", entry.gameId);
    query.bindValue(":modId", entry.modId);
    query.bindValue(":type", static_cast<int>(entry.type));
    query.bindValue(":fileMd5", QString::fromStdString(entry.fileMd5));
    query.bindValue(":fileCrc32", QString::fromStdString(entry.fileCrc32));
    query.bindValue(":sourceDirectory",
                    QString::fromStdString(entry.sourceDirectory));
    query.bindValue(":contentPath", QString::fromStdString(entry.contentPath));
    query.bindValue(":createdAt",
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::system_clock::now().time_since_epoch())
                    .count());

    if (!query.exec()) {
      spdlog::error("Failed to create library entry: {}",
                    query.lastError().text().toStdString());
      query.finish();
      return false;
    }

    entry.id = query.lastInsertId().toInt();

    query.finish();
    emit libraryEntryCreated(entry);
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

    if (!entry.contentId.empty()) {
      if (needAND) {
        whereClause += " AND ";
      } else {
        whereClause += " WHERE ";
        needAND = true;
      }
      whereClause += "content_id = :contentId";
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

    if (!entry.fileMd5.empty()) {
      if (needAND) {
        whereClause += " AND ";
      } else {
        whereClause += " WHERE ";
        needAND = true;
      }
      whereClause += "file_md5 = :fileMd5";
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
    if (!entry.contentId.empty()) {
      query.bindValue(":contentId", QString::fromStdString(entry.contentId));
    }
    if (!entry.contentPath.empty()) {
      query.bindValue(":contentPath", QString::fromStdString(entry.contentPath));
    }
    if (!entry.fileMd5.empty()) {
      query.bindValue(":fileMd5", QString::fromStdString(entry.fileMd5));
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

  std::vector<std::string> SqliteLibraryDatabase::getAllContentPaths() {
    QSqlQuery q(getDatabase());
    q.prepare("SELECT content_path FROM library_entries");

    if (!q.exec()) {
      spdlog::error("Failed to get content paths: {}",
                    q.lastError().text().toStdString());
    }

    std::vector<std::string> paths;
    while (q.next()) {
      paths.emplace_back(q.value(0).toString().toStdString());
    }

    return paths;
  }

  bool SqliteLibraryDatabase::createLibraryContentDirectory(
    LibraryContentDirectory &directory) {
    // const auto path = QUrl(QString::fromStdString(directory.path)).toString();

    const QString queryString = "INSERT INTO content_directories (path, "
        "created_at) VALUES (:path, :createdAt);";
    QSqlQuery query(getDatabase());
    query.prepare(queryString);
    query.bindValue(":path", QString::fromStdString(directory.path));
    query.bindValue(":createdAt",
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::system_clock::now().time_since_epoch())
                    .count());

    if (!query.exec()) {
      query.finish();
      return false;
    }

    directory.id = query.lastInsertId().toInt();

    query.finish();
    emit contentDirectoriesUpdated();
    return true;
  }

  bool SqliteLibraryDatabase::updateLibraryContentDirectory(
    LibraryContentDirectory &directory) {
    QSqlQuery q(getDatabase());
    q.prepare("UPDATE content_directories SET path = :path WHERE id = :id");
    q.bindValue(":path", QString::fromStdString(directory.path));
    q.bindValue(":id", directory.id);

    if (!q.exec()) {
      spdlog::error("Failed to update content directory: {}",
                    q.lastError().text().toStdString());
      return false;
    }

    emit contentDirectoriesUpdated();
    return true;
  }

  std::vector<LibraryContentDirectory>
  SqliteLibraryDatabase::getAllLibraryContentDirectories() const {
    QSqlQuery q(getDatabase());
    q.prepare("SELECT * FROM content_directories");

    if (!q.exec()) {
      spdlog::error("Failed to get content directories: {}",
                    q.lastError().text().toStdString());
    }

    std::vector<LibraryContentDirectory> dirs;
    while (q.next()) {
      dirs.emplace_back(createLibraryContentDirectoryFromQuery(q));
    }

    return dirs;
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
    entry.contentId = query.value(2).toString().toStdString();
    entry.platformId = query.value(3).toInt();
    entry.parentEntryId = query.value(4).toInt();
    entry.gameId = query.value(5).toInt();
    entry.modId = query.value(6).toInt();
    entry.activeSaveSlot = query.value(7).toInt();
    entry.type = static_cast<LibraryEntry::EntryType>(query.value(8).toInt());
    entry.fileMd5 = query.value(9).toString().toStdString();
    entry.fileCrc32 = query.value(10).toString().toStdString();
    entry.sourceDirectory = query.value(11).toString().toStdString();
    entry.contentPath = query.value(12).toString().toStdString();
    entry.createdAt = query.value(13).toLongLong();

    return entry;
  }

  LibraryContentDirectory
  SqliteLibraryDatabase::createLibraryContentDirectoryFromQuery(
    const QSqlQuery &query) {
    LibraryContentDirectory directory;
    directory.id = query.value(0).toInt();
    directory.path = query.value(1).toString().toStdString();
    directory.numGameFiles = query.value(2).toInt();

    return directory;
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
    db.setDatabaseName(m_dbFilePath);
    db.open();
    return db;
  }
} // namespace firelight::db

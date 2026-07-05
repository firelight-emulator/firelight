#include <firelight/library/sqlite_user_library.hpp>

#include <firelight/library/library_events.hpp>
#include "firelight/event_dispatcher.hpp"

#include <QSqlError>
#include <QSqlQuery>
#include <QThread>
#include <algorithm>
#include <qfileinfo.h>
#include <spdlog/spdlog.h>
#include <utility>

namespace firelight::library {
  SqliteUserLibraryRepository::SqliteUserLibraryRepository(QString path)
    : m_databasePath(std::move(path)) {
    QSqlQuery createRomFilesTable(getDatabase());
    createRomFilesTable.prepare("CREATE TABLE IF NOT EXISTS content_files("
      "id INTEGER PRIMARY KEY,"
      "file_path TEXT NOT NULL,"
      "file_size INTEGER NOT NULL,"
      "file_md5 TEXT NOT NULL,"
      "file_crc32 TEXT NOT NULL,"
      "in_archive INTEGER NOT NULL DEFAULT 0,"
      "archive_file_path TEXT,"
      "platform_id INTEGER NOT NULL,"
      "content_hash TEXT NOT NULL,"
      "content_type INTEGER NOT NULL DEFAULT 0,"
      "content_directory_id INTEGER NOT NULL DEFAULT -1,"
      "created_at INTEGER NOT NULL);");

    if (!createRomFilesTable.exec()) {
      spdlog::error("Table creation failed: {}",
                    createRomFilesTable.lastError().text().toStdString());
    }

    // A multi-file disc set's member tracks/discs, keyed to the primary
    // ContentFile (the cue/gdi/m3u sheet) in content_files.
    QSqlQuery createDiscMembersTable(getDatabase());
    createDiscMembersTable.prepare("CREATE TABLE IF NOT EXISTS disc_members("
      "id INTEGER PRIMARY KEY,"
      "content_file_id INTEGER NOT NULL,"
      "path TEXT NOT NULL,"
      "role TEXT NOT NULL,"
      "sort_index INTEGER NOT NULL DEFAULT 0,"
      "created_at INTEGER NOT NULL,"
      "UNIQUE (content_file_id, path));");

    if (!createDiscMembersTable.exec()) {
      spdlog::error("Table creation failed: {}",
                    createDiscMembersTable.lastError().text().toStdString());
    }

    QSqlQuery createPatchFilesTable(getDatabase());
    createPatchFilesTable.prepare("CREATE TABLE IF NOT EXISTS patch_files("
      "id INTEGER PRIMARY KEY,"
      "file_path TEXT UNIQUE NOT NULL,"
      "file_size INTEGER NOT NULL,"
      "file_md5 TEXT NOT NULL,"
      "file_crc32 TEXT NOT NULL,"
      "target_md5 TEXT,"
      "patched_md5 TEXT,"
      "patched_content_hash TEXT,"
      "in_archive INTEGER NOT NULL DEFAULT 0,"
      "archive_file_path TEXT,"
      "created_at INTEGER NOT NULL);");

    if (!createPatchFilesTable.exec()) {
      spdlog::error("Table creation failed: {}",
                    createPatchFilesTable.lastError().text().toStdString());
    }

    QSqlQuery createEntriesTable(getDatabase());
    createEntriesTable.prepare("CREATE TABLE IF NOT EXISTS entriesv1("
      "id INTEGER PRIMARY KEY,"
      "display_name TEXT NOT NULL,"
      "content_hash TEXT NOT NULL,"
      "platform_id INTEGER NOT NULL,"
      "active_save_slot INTEGER NOT NULL DEFAULT 1, "
      "hidden INTEGER NOT NULL DEFAULT 0, "
      "favorite INTEGER NOT NULL DEFAULT 0, "
      "icon_1x1_source_url TEXT, "
      "icon_2x3_source_url TEXT,"
      "icon_92x43_source_url TEXT, "
      "boxart_front_source_url TEXT, "
      "boxart_back_source_url TEXT, "
      "clear_logo_source_url TEXT, "
      "hero_image_source_url TEXT, "
      "description TEXT, "
      "release_year INTEGER, "
      "developer TEXT, "
      "publisher TEXT, "
      "genres TEXT, "
      "region_ids TEXT, "
      "retroachievements_set_id INTEGER, "
      "created_at INTEGER NOT NULL);");

    if (!createEntriesTable.exec()) {
      spdlog::error("Table creation failed: {}",
                    createEntriesTable.lastError().text().toStdString());
    }

    QSqlQuery createRunConfigurationsTable(getDatabase());
    createRunConfigurationsTable.prepare(
      "CREATE TABLE IF NOT EXISTS run_configurations("
      "id INTEGER PRIMARY KEY,"
      "type TEXT NOT NULL,"
      "content_hash TEXT NOT NULL,"
      "content_file_id INTEGER NOT NULL,"
      "patch_id INTEGER,"
      "created_at INTEGER NOT NULL,"
      "UNIQUE (type, content_file_id, patch_id),"
      "UNIQUE (type, content_file_id));");

    if (!createRunConfigurationsTable.exec()) {
      spdlog::error("Table creation failed: {}",
                    createEntriesTable.lastError().text().toStdString());
    }

    QSqlQuery createDirectoriesTable(getDatabase());
    createDirectoriesTable.prepare(
      "CREATE TABLE IF NOT EXISTS content_directoriesv1("
      "id INTEGER PRIMARY KEY,"
      "path TEXT UNIQUE NOT NULL,"
      "num_files INTEGER NOT NULL DEFAULT 0,"
      "num_content_files INTEGER NOT NULL DEFAULT 0,"
      "last_modified INTEGER DEFAULT 0,"
      "recursive INTEGER NOT NULL DEFAULT 1,"
      "created_at INTEGER NOT NULL);");

    if (!createDirectoriesTable.exec()) {
      spdlog::error("Table creation failed: {}",
                    createDirectoriesTable.lastError().text().toStdString());
    }

    QSqlQuery createFoldersTable(getDatabase());
    createFoldersTable.prepare("CREATE TABLE IF NOT EXISTS folders("
      "id INTEGER PRIMARY KEY,"
      "display_name TEXT UNIQUE NOT NULL,"
      "description TEXT,"
      "icon_source_url TEXT,"
      "type INTEGER NOT NULL DEFAULT 0,"
      "filter_json TEXT,"
      "color TEXT,"
      "sort_role TEXT,"
      "sort_ascending INTEGER NOT NULL DEFAULT 1,"
      "parent_id INTEGER NOT NULL DEFAULT -1,"
      "position INTEGER NOT NULL DEFAULT 0,"
      "created_at INTEGER NOT NULL);");

    if (!createFoldersTable.exec()) {
      spdlog::error("Table creation failed: {}",
                    createFoldersTable.lastError().text().toStdString());
    }

    QSqlQuery createFolderEntriesTable(getDatabase());
    createFolderEntriesTable.prepare("CREATE TABLE IF NOT EXISTS folder_entries("
      "folder_id INTEGER NOT NULL,"
      "entry_id INTEGER NOT NULL,"
      "created_at INTEGER NOT NULL,"
      "UNIQUE (folder_id, entry_id));");

    if (!createFolderEntriesTable.exec()) {
      spdlog::error("Table creation failed: {}",
                    createFolderEntriesTable.lastError().text().toStdString());
    }

    QSqlQuery createRomFileUniqueIndex(getDatabase());
    createRomFileUniqueIndex.prepare(
      "CREATE UNIQUE INDEX IF NOT EXISTS pathIdx ON content_files(file_path);");

    if (!createRomFileUniqueIndex.exec()) {
      spdlog::error("Query failed: {}",
                    createRomFileUniqueIndex.lastError().text().toStdString());
    }

    // folder_entries is looked up by entry_id (per-entry, in loops). The
    // UNIQUE(folder_id, entry_id) index can't serve entry_id-only lookups, so
    // add a dedicated index to avoid table scans.
    QSqlQuery createFolderEntryIdIndex(getDatabase());
    createFolderEntryIdIndex.prepare(
      "CREATE INDEX IF NOT EXISTS folderEntryEntryIdx ON "
      "folder_entries(entry_id);");

    if (!createFolderEntryIdIndex.exec()) {
      spdlog::error("Query failed: {}",
                    createFolderEntryIdIndex.lastError().text().toStdString());
    }

    // content_hash is the primary lookup key for entries, run configurations and
    // content files (loadEntry + library scanning), so index each.
    for (const auto &indexStmt :
         {"CREATE INDEX IF NOT EXISTS entriesContentHashIdx ON "
          "entriesv1(content_hash);",
          "CREATE INDEX IF NOT EXISTS runConfigContentHashIdx ON "
          "run_configurations(content_hash);",
          "CREATE INDEX IF NOT EXISTS contentFileContentHashIdx ON "
          "content_files(content_hash);"}) {
      QSqlQuery createIndex(getDatabase());
      createIndex.prepare(indexStmt);
      if (!createIndex.exec()) {
        spdlog::error("Index creation failed: {}",
                      createIndex.lastError().text().toStdString());
      }
    }

    // Migrate databases created before these columns existed. CREATE TABLE IF
    // NOT EXISTS won't add columns to an existing table, so add them here;
    // otherwise reads/writes referencing them fail on older databases.
    ensureColumn("content_files", "content_directory_id",
                 "INTEGER NOT NULL DEFAULT -1");
    ensureColumn("folders", "type", "INTEGER NOT NULL DEFAULT 0");
    ensureColumn("folders", "filter_json", "TEXT");
    ensureColumn("folders", "color", "TEXT");
    ensureColumn("folders", "sort_role", "TEXT");
    ensureColumn("folders", "sort_ascending", "INTEGER NOT NULL DEFAULT 1");
    ensureColumn("folders", "parent_id", "INTEGER NOT NULL DEFAULT -1");
    ensureColumn("folders", "position", "INTEGER NOT NULL DEFAULT 0");

    // Give pre-existing content files their directory provenance (new files get
    // it stamped at insert time in create(ContentFile)).
    backfillContentDirectoryIds();

    // The default content directory is guaranteed by UserLibraryService, not
    // seeded here. The scan-time orchestration (content file -> run
    // configuration -> entry) lives in LibraryIngestService, which subscribes to
    // the events published below.
  }

  void SqliteUserLibraryRepository::ensureColumn(
    const QString &table, const QString &column,
    const QString &definition) const {
    QSqlQuery info(getDatabase());
    if (!info.exec(QString("PRAGMA table_info(%1);").arg(table))) {
      spdlog::error("Failed to read schema for {}: {}", table.toStdString(),
                    info.lastError().text().toStdString());
      return;
    }
    while (info.next()) {
      if (info.value("name").toString() == column) {
        return; // already present
      }
    }
    info.finish();

    QSqlQuery alter(getDatabase());
    if (!alter.exec(QString("ALTER TABLE %1 ADD COLUMN %2 %3;")
                    .arg(table, column, definition))) {
      spdlog::error("Failed to add column {}.{}: {}", table.toStdString(),
                    column.toStdString(),
                    alter.lastError().text().toStdString());
    }
  }

  int SqliteUserLibraryRepository::resolveContentDirectoryId(
    const QString &onDiskPath) {
    int bestId = -1;
    int bestLen = -1;
    for (const auto &dir: getContentDirectories()) {
      if (onDiskPath.startsWith(QString::fromStdString(dir.path)) &&
          static_cast<int>(dir.path.length()) > bestLen) {
        bestId = dir.id;
        bestLen = static_cast<int>(dir.path.length());
      }
    }
    return bestId;
  }

  void SqliteUserLibraryRepository::backfillContentDirectoryIds() {
    if (getContentDirectories().empty()) {
      return;
    }
    for (const auto &cf: getContentFiles()) {
      if (cf.m_contentDirectoryId >= 0) {
        continue; // already stamped
      }
      const auto onDisk = QString::fromStdString(
        cf.m_inArchive ? cf.m_archivePathName : cf.m_filePath);
      const int dirId = resolveContentDirectoryId(onDisk);
      if (dirId < 0) {
        continue;
      }
      QSqlQuery upd(getDatabase());
      upd.prepare("UPDATE content_files SET content_directory_id = :dirId "
        "WHERE id = :id;");
      upd.bindValue(":dirId", dirId);
      upd.bindValue(":id", cf.m_id);
      if (!upd.exec()) {
        spdlog::error("Failed to backfill content_directory_id for file {}: {}",
                      cf.m_id, upd.lastError().text().toStdString());
      }
    }
  }

  SqliteUserLibraryRepository::~SqliteUserLibraryRepository() {
    for (const auto &name: QSqlDatabase::connectionNames()) {
      if (name.startsWith(DATABASE_PREFIX)) {
        QSqlDatabase::removeDatabase(name);
      }
    }
  }

  bool SqliteUserLibraryRepository::create(FolderInfo &folder) {
    // New folders append to the end of their parent's ordering.
    folder.position = nextFolderPosition(folder.parentId);

    QSqlQuery query(getDatabase());
    query.prepare("INSERT INTO folders("
      "display_name, "
      "description, "
      "icon_source_url, "
      "type, "
      "filter_json, "
      "color, "
      "sort_role, "
      "sort_ascending, "
      "parent_id, "
      "position, "
      "created_at) VALUES"
      "(:displayName, :description, :iconSourceUrl, :type, :filterJson, "
      ":color, :sortRole, :sortAscending, :parentId, :position, :createdAt);");

    query.bindValue(":displayName", QString::fromStdString(folder.displayName));
    query.bindValue(":description", QString::fromStdString(folder.description));
    query.bindValue(":iconSourceUrl",
                    QString::fromStdString(folder.iconSourceUrl));
    query.bindValue(":type", folder.type);
    query.bindValue(":filterJson", QString::fromStdString(folder.filterJson));
    query.bindValue(":color", QString::fromStdString(folder.color));
    query.bindValue(":sortRole", QString::fromStdString(folder.sortRole));
    query.bindValue(":sortAscending", folder.sortAscending ? 1 : 0);
    query.bindValue(":parentId", folder.parentId);
    query.bindValue(":position", folder.position);
    query.bindValue(":createdAt", QDateTime::currentSecsSinceEpoch());

    if (!query.exec()) {
      spdlog::error("Failed to create folder: {}",
                    query.lastError().text().toStdString());
      return false;
    }

    folder.id = query.lastInsertId().toInt();

    return true;
  }

  int SqliteUserLibraryRepository::nextFolderPosition(int parentId) {
    QSqlQuery query(getDatabase());
    query.prepare("SELECT COALESCE(MAX(position), -1) + 1 AS next "
      "FROM folders WHERE parent_id = :parentId;");
    query.bindValue(":parentId", parentId);
    if (query.exec() && query.next()) {
      return query.value("next").toInt();
    }
    return 0;
  }

  bool SqliteUserLibraryRepository::create(FolderEntryInfo &folderEntry) {
    QSqlQuery query(getDatabase());
    query.prepare("INSERT INTO folder_entries("
      "folder_id, "
      "entry_id, "
      "created_at) VALUES"
      "(:folderId, :entryId, :createdAt);");

    query.bindValue(":folderId", folderEntry.folderId);
    query.bindValue(":entryId", folderEntry.entryId);
    query.bindValue(":createdAt", QDateTime::currentSecsSinceEpoch());

    if (!query.exec()) {
      spdlog::error("Failed to create folder entry: {}",
                    query.lastError().text().toStdString());
      return false;
    }

    return true;
  }

  std::vector<FolderInfo>
  SqliteUserLibraryRepository::listFolders() {
    QSqlQuery query(getDatabase());
    // Order within each parent scope by the manual position, so callers get
    // folders in user order (and can group by parent_id for the nested tree).
    query.prepare("SELECT * FROM folders ORDER BY parent_id, position, id");

    if (!query.exec()) {
      spdlog::error("Failed to get folders: {}",
                    query.lastError().text().toStdString());
      return {};
    }

    std::vector<FolderInfo> folders;
    while (query.next()) {
      folders.emplace_back(FolderInfo{
        .id = query.value("id").toInt(),
        .displayName = query.value("display_name").toString().toStdString(),
        .description = query.value("description").toString().toStdString(),
        .iconSourceUrl =
        query.value("icon_source_url").toString().toStdString(),
        .type = query.value("type").toInt(),
        .filterJson = query.value("filter_json").toString().toStdString(),
        .color = query.value("color").toString().toStdString(),
        .sortRole = query.value("sort_role").toString().toStdString(),
        .sortAscending = query.value("sort_ascending").toBool(),
        .parentId = query.value("parent_id").toInt(),
        .position = query.value("position").toInt(),
        .createdAt = query.value("created_at").toULongLong()
      });
    }

    return folders;
  }

  bool SqliteUserLibraryRepository::deleteFolder(int folderId) {
    QSqlQuery query(getDatabase());
    query.prepare("DELETE FROM folders WHERE id = :folderId;");
    query.bindValue(":folderId", folderId);

    if (!query.exec()) {
      spdlog::error("Failed to delete folder with ID {}: {}", folderId,
                    query.lastError().text().toStdString());
      return false;
    }

    query.finish();

    QSqlQuery deleteEntriesQuery(getDatabase());
    deleteEntriesQuery.prepare(
      "DELETE FROM folder_entries WHERE folder_id = :folderId;");
    deleteEntriesQuery.bindValue(":folderId", folderId);

    if (!deleteEntriesQuery.exec()) {
      spdlog::error("Failed to delete entries for folder ID {}: {}", folderId,
                    deleteEntriesQuery.lastError().text().toStdString());
      return false;
    }

    return true;
  }

  bool SqliteUserLibraryRepository::update(FolderInfo &folder) {
    if (folder.id <= 0) {
      spdlog::error("Cannot update folder with invalid ID: {}", folder.id);
      return false;
    }

    QSqlQuery query(getDatabase());
    // Ordering (parent_id/position) is managed by reorderFolders/setFolderParent,
    // not here, so a stale FolderInfo can't clobber the user's arrangement.
    query.prepare("UPDATE folders SET "
      "display_name = :displayName, "
      "description = :description, "
      "icon_source_url = :iconSourceUrl, "
      "type = :type, "
      "filter_json = :filterJson, "
      "color = :color, "
      "sort_role = :sortRole, "
      "sort_ascending = :sortAscending "
      "WHERE id = :folderId;");

    query.bindValue(":folderId", folder.id);
    query.bindValue(":displayName", QString::fromStdString(folder.displayName));
    query.bindValue(":description", QString::fromStdString(folder.description));
    query.bindValue(":iconSourceUrl",
                    QString::fromStdString(folder.iconSourceUrl));
    query.bindValue(":type", folder.type);
    query.bindValue(":filterJson", QString::fromStdString(folder.filterJson));
    query.bindValue(":color", QString::fromStdString(folder.color));
    query.bindValue(":sortRole", QString::fromStdString(folder.sortRole));
    query.bindValue(":sortAscending", folder.sortAscending ? 1 : 0);

    if (!query.exec() || query.numRowsAffected() == 0) {
      spdlog::error("Failed to update folder with ID {}: {}", folder.id,
                    query.lastError().text().toStdString());
      return false;
    }

    return true;
  }

  bool SqliteUserLibraryRepository::reorderFolders(
    const int parentId, const std::vector<int> &orderedFolderIds) {
    auto db = getDatabase();
    db.transaction();
    for (size_t i = 0; i < orderedFolderIds.size(); ++i) {
      QSqlQuery query(db);
      query.prepare("UPDATE folders SET position = :position "
        "WHERE id = :folderId AND parent_id = :parentId;");
      query.bindValue(":position", static_cast<int>(i));
      query.bindValue(":folderId", orderedFolderIds[i]);
      query.bindValue(":parentId", parentId);
      if (!query.exec()) {
        spdlog::error("Failed to reorder folder {}: {}", orderedFolderIds[i],
                      query.lastError().text().toStdString());
        db.rollback();
        return false;
      }
    }
    return db.commit();
  }

  bool SqliteUserLibraryRepository::setFolderParent(const int folderId,
                                                    const int newParentId) {
    QSqlQuery query(getDatabase());
    // Moving to a new parent appends the folder to the end of that parent's
    // ordering.
    query.prepare("UPDATE folders SET parent_id = :parentId, position = :position "
      "WHERE id = :folderId;");
    query.bindValue(":parentId", newParentId);
    query.bindValue(":position", nextFolderPosition(newParentId));
    query.bindValue(":folderId", folderId);
    if (!query.exec() || query.numRowsAffected() == 0) {
      spdlog::error("Failed to set parent of folder {} to {}: {}", folderId,
                    newParentId, query.lastError().text().toStdString());
      return false;
    }
    return true;
  }

  bool SqliteUserLibraryRepository::deleteFolderEntry(FolderEntryInfo &info) {
    QSqlQuery query(getDatabase());
    query.prepare("DELETE FROM folder_entries WHERE folder_id = :folderId AND "
      "entry_id = :entryId;");
    query.bindValue(":folderId", info.folderId);
    query.bindValue(":entryId", info.entryId);

    if (!query.exec()) {
      spdlog::error("Failed to delete folder entry: {}",
                    query.lastError().text().toStdString());
      return false;
    }

    return true;
  }

  bool SqliteUserLibraryRepository::update(Entry &entry) {
    QSqlQuery query(getDatabase());
    query.prepare("UPDATE entriesv1 SET "
      "display_name = :displayName, "
      "active_save_slot = :activeSaveSlot, "
      "hidden = :hidden, "
      "favorite = :favorite WHERE id = :id;");

    query.bindValue(":id", entry.id);
    query.bindValue(":displayName", QString::fromStdString(entry.displayName));
    query.bindValue(":activeSaveSlot", entry.activeSaveSlot);
    query.bindValue(":hidden", entry.hidden ? 1 : 0);
    query.bindValue(":favorite", entry.favorite ? 1 : 0);
    if (!query.exec() || query.numRowsAffected() == 0) {
      spdlog::error("Failed to update entry with ID {}: {}", entry.id,
                    query.lastError().text().toStdString());
      return false;
    }

    EventDispatcher::instance().publish(EntryUpdatedEvent{.entryId = entry.id});
    return true;
  }

  bool SqliteUserLibraryRepository::deleteContentDirectory(int id) {
    QSqlQuery selectQuery(getDatabase());
    selectQuery.prepare("SELECT * FROM content_directoriesv1 WHERE id = :id;");
    selectQuery.bindValue(":id", id);

    if (!selectQuery.exec()) {
      spdlog::error("Failed to get content directory with ID {}: {}", id,
                    selectQuery.lastError().text().toStdString());
      return false;
    }

    if (!selectQuery.next()) {
      return true;
    }

    const auto path = selectQuery.value("path").toString();
    selectQuery.finish();

    QSqlQuery query(getDatabase());
    query.prepare("DELETE FROM content_directoriesv1 WHERE id = :id;");

    query.bindValue(":id", id);
    if (!query.exec()) {
      spdlog::error("Failed to delete content directory with ID {}: {}", id,
                    query.lastError().text().toStdString());
      return false;
    }

    for (const auto &rom: getContentFiles()) {
      auto romPath = QString::fromStdString(rom.m_filePath);
      if (rom.m_inArchive) {
        romPath = QString::fromStdString(rom.m_archivePathName);
      }

      if (romPath.startsWith(path)) {
        auto found = false;
        auto contentPaths = getContentDirectories();
        for (const auto &contentPath: contentPaths) {
          if (romPath.startsWith(QString::fromStdString(contentPath.path))) {
            found = true;
            break;
          }
        }

        if (!found) {
          deleteContentFile(rom.m_id);
        }
      }
    }

    EventDispatcher::instance().publish(
      ContentDirectoryRemovedEvent{.id = id, .path = path.toStdString()});
    return true;
  }

  bool SqliteUserLibraryRepository::create(ContentFile &romFile) {
    // Stamp folder-source provenance: resolve which content directory this file
    // lives under (by on-disk path — the archive path for archived content).
    if (romFile.m_contentDirectoryId < 0) {
      const auto onDisk = QString::fromStdString(
        romFile.m_inArchive ? romFile.m_archivePathName : romFile.m_filePath);
      romFile.m_contentDirectoryId = resolveContentDirectoryId(onDisk);
    }

    const QString queryString = "INSERT INTO content_files ("
        "file_path, "
        "file_size, "
        "file_md5, "
        "file_crc32,"
        "in_archive, "
        "archive_file_path, "
        "platform_id, "
        "content_hash, "
        "content_type, "
        "content_directory_id, "
        "created_at) VALUES"
        "(:filePath, :fileSize, :fileMd5, :fileCrc32, "
        ":inArchive, :archiveFilePath, :platformId, "
        ":contentHash, :contentType, :contentDirectoryId, :createdAt);";
    QSqlQuery query(getDatabase());
    query.prepare(queryString);
    query.bindValue(":filePath", QString::fromStdString(romFile.m_filePath));
    query.bindValue(":fileSize", QVariant::fromValue(romFile.m_fileSizeBytes));
    query.bindValue(":fileMd5", QString::fromStdString(romFile.m_fileMd5));
    query.bindValue(":fileCrc32", QString::fromStdString(romFile.m_fileCrc32));
    query.bindValue(":inArchive", romFile.m_inArchive);
    query.bindValue(":archiveFilePath",
                    QString::fromStdString(romFile.m_archivePathName));
    query.bindValue(":platformId", romFile.m_platformId);
    query.bindValue(":contentHash",
                    QString::fromStdString(romFile.m_contentHash));
    query.bindValue(":contentType", static_cast<int>(romFile.m_type));
    query.bindValue(":contentDirectoryId", romFile.m_contentDirectoryId);
    query.bindValue(":createdAt",
                    QVariant::fromValue(
                      std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch())
                      .count()));

    if (!query.exec()) {
      spdlog::error("Failed to add rom file with path {}: {}", romFile.m_filePath,
                    query.lastError().text().toStdString());
      return false;
    }

    romFile.m_id = query.lastInsertId().toInt();

    EventDispatcher::instance().publish(
      ContentFileAddedEvent{.id = romFile.m_id,
                            .filePath = romFile.m_filePath,
                            .platformId = romFile.m_platformId,
                            .contentHash = romFile.m_contentHash});
    return true;
  }

  std::optional<ContentFile> SqliteUserLibraryRepository::getContentFileWithPathAndSize(
    const std::string &filePath, const size_t fileSizeBytes, const bool inArchive) {
    const QString queryString =
        "SELECT * FROM content_files WHERE file_path = :filePath AND file_size = "
        ":fileSize AND in_archive = :inArchive;";
    QSqlQuery query(getDatabase());
    query.prepare(queryString);
    query.bindValue(":filePath", QString::fromStdString(filePath));
    query.bindValue(":fileSize", QVariant::fromValue(fileSizeBytes));
    query.bindValue(":inArchive", inArchive);

    if (!query.exec()) {
      spdlog::error("Failed to get rom file with path {}: {}", filePath,
                    query.lastError().text().toStdString());
    }

    if (!query.next()) {
      return std::nullopt;
    }

    return {
      ContentFile{
        .m_id = query.value("id").toInt(),
        .m_type =
        static_cast<ContentType>(query.value("content_type").toInt()),
        .m_fileSizeBytes =
        static_cast<size_t>(query.value("file_size").toLongLong()),
        .m_filePath = query.value("file_path").toString().toStdString(),
        .m_fileMd5 = query.value("file_md5").toString().toStdString(),
        .m_inArchive = query.value("in_archive").toBool(),
        .m_archivePathName =
        query.value("archive_file_path").toString().toStdString(),
        .m_platformId = query.value("platform_id").toInt(),
        .m_contentHash = query.value("content_hash").toString().toStdString(),
        .m_contentDirectoryId =
        query.value("content_directory_id").toInt(),
      }
    };
  }

  bool SqliteUserLibraryRepository::deleteContentFile(int id) {
    QSqlQuery query(getDatabase());
    query.prepare("SELECT * FROM content_files WHERE id = :id;");
    query.bindValue(":id", id);

    if (!query.exec()) {
      spdlog::error("Failed to get rom file with ID {}: {}", id,
                    query.lastError().text().toStdString());
      return false;
    }

    if (!query.next()) {
      return true;
    }

    const auto contentHash = query.value("content_hash").toString();
    query.finish();

    QSqlQuery deleteQuery(getDatabase());
    deleteQuery.prepare("DELETE FROM content_files WHERE id = :id;");
    deleteQuery.bindValue(":id", id);

    if (!deleteQuery.exec()) {
      spdlog::error("Failed to delete rom file with ID {}: {}", id,
                    deleteQuery.lastError().text().toStdString());
      return false;
    }

    QSqlQuery deleteRunConfigsQuery(getDatabase());
    deleteRunConfigsQuery.prepare("DELETE FROM run_configurations WHERE type = "
      "\"rom\" AND content_file_id = :contentFileId;");
    deleteRunConfigsQuery.bindValue(":contentFileId", id);

    if (!deleteRunConfigsQuery.exec()) {
      spdlog::error("Failed to delete run configurations for rom file ID {}: {} ",
                    id, deleteRunConfigsQuery.lastError().text().toStdString());
      return false;
    }

    QSqlQuery deleteDiscMembersQuery(getDatabase());
    deleteDiscMembersQuery.prepare(
      "DELETE FROM disc_members WHERE content_file_id = :id;");
    deleteDiscMembersQuery.bindValue(":id", id);

    if (!deleteDiscMembersQuery.exec()) {
      spdlog::error("Failed to delete disc members for content file ID {}: {} ",
                    id, deleteDiscMembersQuery.lastError().text().toStdString());
      return false;
    }

    EventDispatcher::instance().publish(
      RunConfigurationDeletedEvent{.contentHash = contentHash.toStdString()});
    return true;
  }

  Entry SqliteUserLibraryRepository::deserializeEntry(const QSqlQuery &query) {
    return Entry{
      .id = query.value("id").toInt(),
      .displayName = query.value("display_name").toString().toStdString(),
      .contentHash = query.value("content_hash").toString().toStdString(),
      .platformId = query.value("platform_id").toUInt(),
      .activeSaveSlot = query.value("active_save_slot").toUInt(),
      .hidden = query.value("hidden").toBool(),
      .favorite = query.value("favorite").toBool(),
      .icon1x1SourceUrl =
      query.value("icon_1x1_source_url").toString().toStdString(),
      .boxartFrontSourceUrl =
      query.value("boxart_front_source_url").toString().toStdString(),
      .boxartBackSourceUrl =
      query.value("boxart_back_source_url").toString().toStdString(),
      .description = query.value("description").toString().toStdString(),
      .releaseYear = query.value("release_year").toUInt(),
      .developer = query.value("developer").toString().toStdString(),
      .publisher = query.value("publisher").toString().toStdString(),
      .genres = query.value("genres").toString().toStdString(),
      .regionIds = query.value("region_ids").toString().toStdString(),
      .retroachievementsSetId =
      query.value("retroachievements_set_id").toUInt(),
      .createdAt = query.value("created_at").toULongLong()
    };
  }

  std::vector<Entry> SqliteUserLibraryRepository::getEntries(int offset, int limit) {
    const QString queryString = R"(
            SELECT e.*,
                   CASE
                       WHEN EXISTS (SELECT 1 FROM content_files rf WHERE rf.content_hash = e.content_hash)
                       THEN 1
                       ELSE 0
                   END AS has_rom
            FROM entriesv1 e
            ORDER BY e.display_name ASC;
        )";

    QSqlQuery query(getDatabase());
    query.prepare(queryString);

    if (!query.exec()) {
      spdlog::error("Failed to get entries: {}",
                    query.lastError().text().toStdString());
      return {};
    }

    std::vector<Entry> entries;

    while (query.next()) {
      entries.emplace_back(deserializeEntry(query));
    }

    query.finish();

    for (auto &entry: entries) {
      QSqlQuery folderQuery(getDatabase());
      folderQuery.prepare(
        "SELECT folder_id FROM folder_entries WHERE entry_id = :entryId;");
      folderQuery.bindValue(":entryId", entry.id);

      if (folderQuery.exec()) {
        while (folderQuery.next()) {
          entry.folderIds.push_back(folderQuery.value("folder_id").toInt());
        }
      } else {
        spdlog::error("Failed to get folder IDs for entry {}: {}", entry.id,
                      folderQuery.lastError().text().toStdString());
      }

      populateEntryProvenance(entry);
    }

    return entries;
  }

  std::optional<Entry> SqliteUserLibraryRepository::getEntry(const int entryId) {
    const QString queryString =
        "SELECT * FROM entriesv1 WHERE id = :entryId LIMIT 1;";
    QSqlQuery query(getDatabase());
    query.prepare(queryString);
    query.bindValue(":entryId", entryId);

    if (!query.exec()) {
      spdlog::error("Failed to get entry: {}",
                    query.lastError().text().toStdString());
      return {};
    }

    if (!query.next()) {
      spdlog::error("Failed to get entry: {}",
                    query.lastError().text().toStdString());
      return {};
    }

    auto entry = deserializeEntry(query);

    query.finish();

    QSqlQuery folderQuery(getDatabase());
    folderQuery.prepare(
      "SELECT folder_id FROM folder_entries WHERE entry_id = :entryId;");
    folderQuery.bindValue(":entryId", entry.id);

    if (folderQuery.exec()) {
      while (folderQuery.next()) {
        entry.folderIds.push_back(folderQuery.value("folder_id").toInt());
      }
    } else {
      spdlog::error("Failed to get folder IDs for entry {}: {}", entry.id,
                    folderQuery.lastError().text().toStdString());
    }

    populateEntryProvenance(entry);

    return entry;
  }

  std::optional<Entry>
  SqliteUserLibraryRepository::getEntryWithContentHash(const std::string &contentHash) {
    const QString queryString =
        "SELECT * FROM entriesv1 WHERE content_hash = :contentHash LIMIT 1;";
    QSqlQuery query(getDatabase());
    query.prepare(queryString);
    query.bindValue(":contentHash", QString::fromStdString(contentHash));

    if (!query.exec()) {
      spdlog::error("Failed to get entry with content hash {}: {}", contentHash,
                    query.lastError().text().toStdString());
      return {};
    }

    if (!query.next()) {
      return {};
    }

    auto entry = deserializeEntry(query);

    query.finish();

    QSqlQuery folderQuery(getDatabase());
    folderQuery.prepare(
      "SELECT folder_id FROM folder_entries WHERE entry_id = :entryId;");
    folderQuery.bindValue(":entryId", entry.id);

    if (folderQuery.exec()) {
      while (folderQuery.next()) {
        entry.folderIds.push_back(folderQuery.value("folder_id").toInt());
      }
    } else {
      spdlog::error("Failed to get folder IDs for entry {}: {}", entry.id,
                    folderQuery.lastError().text().toStdString());
    }

    populateEntryProvenance(entry);

    return entry;
  }

  void SqliteUserLibraryRepository::populateEntryProvenance(Entry &entry) {
    QSqlQuery query(getDatabase());
    query.prepare("SELECT content_directory_id, file_path, in_archive, "
      "archive_file_path FROM content_files WHERE content_hash = :contentHash;");
    query.bindValue(":contentHash", QString::fromStdString(entry.contentHash));
    if (!query.exec()) {
      spdlog::error("Failed to get content provenance for entry {}: {}",
                    entry.id, query.lastError().text().toStdString());
      return;
    }
    while (query.next()) {
      const int dirId = query.value("content_directory_id").toInt();
      if (dirId >= 0 && std::find(entry.contentDirectoryIds.begin(),
                                  entry.contentDirectoryIds.end(), dirId) ==
          entry.contentDirectoryIds.end()) {
        entry.contentDirectoryIds.push_back(dirId);
      }
      const auto path = query.value("in_archive").toBool()
                          ? query.value("archive_file_path").toString()
                          : query.value("file_path").toString();
      entry.contentPaths.push_back(path.toStdString());
    }
  }

  std::vector<RunConfiguration>
  SqliteUserLibraryRepository::getRunConfigurations(const std::string &contentHash) {
    const auto queryString =
        "SELECT * FROM run_configurations WHERE content_hash = :contentHash;";
    QSqlQuery query(getDatabase());
    query.prepare(queryString);

    query.bindValue(":contentHash", QString::fromStdString(contentHash));
    if (!query.exec()) {
      spdlog::error("Failed to get run configurations: {}",
                    query.lastError().text().toStdString());
    }

    std::vector<RunConfiguration> runConfigurations;
    while (query.next()) {
      RunConfiguration runConfiguration{
        .id = query.value("id").toInt(),
        .type = query.value("type").toString().toStdString(),
        .contentHash = contentHash,
        .contentFileId = query.value("content_file_id").toInt(),
        .patchId = query.value("patch_id").toInt(),
        .createdAt = query.value("created_at").toUInt()
      };

      runConfigurations.push_back(runConfiguration);
    }

    return runConfigurations;
  }

  std::vector<ContentFile> SqliteUserLibraryRepository::getContentFiles() {
    const QString queryString = "SELECT * FROM content_files;";
    QSqlQuery query(getDatabase());
    query.prepare(queryString);

    if (!query.exec()) {
      spdlog::error("Failed to get rom files: {}",
                    query.lastError().text().toStdString());
    }

    std::vector<ContentFile> romFiles;

    while (query.next()) {
      romFiles.emplace_back(ContentFile{
        .m_id = query.value("id").toInt(),
        .m_type =
        static_cast<ContentType>(query.value("content_type").toInt()),
        .m_fileSizeBytes =
        static_cast<size_t>(query.value("file_size").toLongLong()),
        .m_filePath = query.value("file_path").toString().toStdString(),
        .m_fileMd5 = query.value("file_md5").toString().toStdString(),
        .m_inArchive = query.value("in_archive").toBool(),
        .m_archivePathName =
        query.value("archive_file_path").toString().toStdString(),
        .m_platformId = query.value("platform_id").toInt(),
        .m_contentHash = query.value("content_hash").toString().toStdString(),
        .m_contentDirectoryId =
        query.value("content_directory_id").toInt(),
      });
    }

    return romFiles;
  }

  std::optional<ContentFile> SqliteUserLibraryRepository::getContentFile(int id) {
    auto queryString = "SELECT * FROM content_files WHERE id = :id LIMIT 1;";
    QSqlQuery query(getDatabase());
    query.prepare(queryString);
    query.bindValue(":id", id);

    if (!query.exec()) {
      spdlog::error("Failed to get rom file with id {}: {}", id,
                    query.lastError().text().toStdString());
    }

    if (!query.next()) {
      return std::nullopt;
    }

    return ContentFile{
      .m_id = query.value("id").toInt(),
      .m_type = static_cast<ContentType>(query.value("content_type").toInt()),
      .m_fileSizeBytes =
      static_cast<size_t>(query.value("file_size").toLongLong()),
      .m_filePath = query.value("file_path").toString().toStdString(),
      .m_fileMd5 = query.value("file_md5").toString().toStdString(),
      .m_inArchive = query.value("in_archive").toBool(),
      .m_archivePathName =
      query.value("archive_file_path").toString().toStdString(),
      .m_platformId = query.value("platform_id").toInt(),
      .m_contentHash = query.value("content_hash").toString().toStdString(),
      .m_contentDirectoryId = query.value("content_directory_id").toInt(),
    };
  }

  bool SqliteUserLibraryRepository::create(DiscMember &member) {
    QSqlQuery query(getDatabase());
    query.prepare("INSERT OR IGNORE INTO disc_members ("
      "content_file_id, path, role, sort_index, created_at) VALUES "
      "(:contentFileId, :path, :role, :sortIndex, :createdAt);");
    query.bindValue(":contentFileId", member.m_contentFileId);
    query.bindValue(":path", QString::fromStdString(member.m_path));
    query.bindValue(":role", QString::fromStdString(member.m_role));
    query.bindValue(":sortIndex", member.m_sortIndex);
    query.bindValue(":createdAt", QDateTime::currentMSecsSinceEpoch());

    if (!query.exec()) {
      spdlog::error("Failed to add disc member {}: {}", member.m_path,
                    query.lastError().text().toStdString());
      return false;
    }

    member.m_id = query.lastInsertId().toInt();
    return true;
  }

  std::optional<PatchFile> SqliteUserLibraryRepository::getPatchFile(int id) {
    auto queryString = "SELECT * FROM patch_files WHERE id = :id LIMIT 1;";
    QSqlQuery query(getDatabase());
    query.prepare(queryString);
    query.bindValue(":id", id);

    if (!query.exec()) {
      spdlog::error("Failed to get patch file with id {}: {}", id,
                    query.lastError().text().toStdString());
    }

    if (!query.next()) {
      return std::nullopt;
    }

    PatchFile patchFile;
    patchFile.m_filePath = query.value("file_path").toString().toStdString();
    patchFile.m_fileSize = query.value("file_size").toInt();
    patchFile.m_fileMd5 = query.value("file_md5").toString().toStdString();

    return {patchFile};
  }

  void SqliteUserLibraryRepository::create(PatchFile &file) {
    const QString queryString =
        "INSERT OR IGNORE INTO patch_files ("
        "file_path, "
        "file_size, "
        "file_md5, "
        "file_crc32, "
        "target_md5, "
        "patched_md5, "
        "patched_content_hash, "
        "in_archive, "
        "archive_file_path, "
        "created_at) VALUES"
        "(:filePath, :fileSize, :fileMd5, :fileCrc32, :targetMd5, "
        ":patchedMd5, "
        ":patchedContentHash, :inArchive, :archiveFilePath, :createdAt);";
    QSqlQuery query(getDatabase());
    query.prepare(queryString);
    query.bindValue(":filePath", QString::fromStdString(file.m_filePath));
    query.bindValue(":fileSize", QVariant::fromValue(file.m_fileSize));
    query.bindValue(":fileMd5", QString::fromStdString(file.m_fileMd5));
    query.bindValue(":fileCrc32", QString::fromStdString(file.m_fileCrc32));
    query.bindValue(":targetMd5", QString::fromStdString(file.m_targetFileMd5));
    query.bindValue(":patchedMd5", QString::fromStdString(file.m_patchedMd5));
    query.bindValue(":patchedContentHash",
                    QString::fromStdString(file.m_patchedContentHash));
    query.bindValue(":inArchive", file.m_inArchive);
    query.bindValue(":archiveFilePath",
                    QString::fromStdString(file.m_archiveFilePath));
    query.bindValue(":createdAt",
                    QVariant::fromValue(
                      std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch())
                      .count()));

    if (!query.exec()) {
      spdlog::error("Failed to add patch file with path {}: {}", file.m_filePath,
                    query.lastError().text().toStdString());
    }
  }

  std::vector<ContentDirectory> SqliteUserLibraryRepository::getContentDirectories() {
    const QString queryString = "SELECT * FROM content_directoriesv1;";
    QSqlQuery query(getDatabase());
    query.prepare(queryString);

    if (!query.exec()) {
      spdlog::error("Failed to get directories: {}",
                    query.lastError().text().toStdString());
    }

    std::vector<ContentDirectory> directories;

    while (query.next()) {
      directories.emplace_back(ContentDirectory{
        .id = query.value("id").toInt(),
        .path = query.value("path").toString().toStdString(),
        .numFiles = query.value("num_files").toInt(),
        .numContentFiles = query.value("num_content_files").toInt(),
        .lastModifiedEpochMs = query.value("last_modified").toULongLong(),
        .recursive = query.value("recursive").toBool(),
        .createdAt = query.value("created_at").toUInt()
      });
    }

    return directories;
  }

  bool SqliteUserLibraryRepository::create(ContentDirectory &directory) {
    const QString queryString = "INSERT OR IGNORE INTO content_directoriesv1 ("
        "path, "
        "created_at) VALUES"
        "(:path, :createdAt);";
    QSqlQuery query(getDatabase());
    query.prepare(queryString);
    query.bindValue(":path", QString::fromStdString(directory.path));
    query.bindValue(":createdAt",
                    QVariant::fromValue(
                      std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch())
                      .count()));

    if (!query.exec()) {
      spdlog::error("Failed to add directory with path {}: {}", directory.path,
                    query.lastError().text().toStdString());
      return false;
    }

    if (query.numRowsAffected() == 0) {
      return false;
    }

    directory.id = query.lastInsertId().toInt();
    EventDispatcher::instance().publish(
      ContentDirectoryAddedEvent{.id = directory.id, .path = directory.path});
    return true;
  }

  bool SqliteUserLibraryRepository::update(const ContentDirectory &directory) {
    QSqlQuery selectQuery(getDatabase());
    selectQuery.prepare("SELECT * FROM content_directoriesv1 WHERE id = :id;");
    selectQuery.bindValue(":id", directory.id);

    if (!selectQuery.exec()) {
      spdlog::error("Failed to get content directory with ID {}: {}",
                    directory.id, selectQuery.lastError().text().toStdString());
      return false;
    }

    if (!selectQuery.next()) {
      return true;
    }

    const auto oldPath = selectQuery.value("path").toString();
    selectQuery.finish();

    QSqlQuery q(getDatabase());
    q.prepare("UPDATE content_directoriesv1 SET path = :path WHERE id = :id");
    q.bindValue(":path", QString::fromStdString(directory.path));
    q.bindValue(":id", directory.id);

    if (!q.exec()) {
      spdlog::error("Failed to update content directory: {}",
                    q.lastError().text().toStdString());
      return false;
    }

    EventDispatcher::instance().publish(
      ContentDirectoryUpdatedEvent{.id = directory.id,
                                   .oldPath = oldPath.toStdString(),
                                   .newPath = directory.path});
    return true;
  }

  void SqliteUserLibraryRepository::createRunConfiguration(const int contentFileId,
                                                    const std::string &path,
                                                    const int platformId,
                                                    const std::string &contentHash) {
    const QString insertQueryString =
        "INSERT OR IGNORE INTO run_configurations "
        "(type, content_hash, content_file_id, created_at) "
        "VALUES (:type, :contentHash, :contentFileId, :createdAt);";

    QSqlQuery query(getDatabase());
    query.prepare(insertQueryString);

    query.bindValue(":type", "rom");
    query.bindValue(":contentHash", QString::fromStdString(contentHash));
    query.bindValue(":contentFileId", contentFileId);
    query.bindValue(":createdAt", QDateTime::currentSecsSinceEpoch());

    if (!query.exec()) {
      spdlog::error("Failed to create run configuration: {}",
                    query.lastError().text().toStdString());
    }

    EventDispatcher::instance().publish(
      RunConfigurationCreatedEvent{.id = query.lastInsertId().toInt(),
                                   .filePath = path,
                                   .platformId = platformId,
                                   .contentHash = contentHash});
  }

  bool SqliteUserLibraryRepository::createEntry(Entry &entry) {
    QSqlQuery selectQuery(getDatabase());
    selectQuery.prepare(
      "SELECT * FROM entriesv1 WHERE content_hash = :contentHash;");
    selectQuery.bindValue(":contentHash", QString::fromStdString(entry.contentHash));
    if (!selectQuery.exec()) {
      spdlog::error("Failed to check for existing entry with content hash {}: {}",
                    entry.contentHash,
                    selectQuery.lastError().text().toStdString());
      return false;
    }

    if (selectQuery.next()) {
      spdlog::debug(
        "Entry with content hash {} already exists, skipping creation",
        entry.contentHash);
      return false;
    }

    const QString entryQueryString =
        "INSERT INTO entriesv1 ("
        "display_name, "
        "content_hash, "
        "platform_id, "
        "created_at) VALUES"
        "(:displayName, :contentHash, :platformId, :createdAt);";
    QSqlQuery entryQuery(getDatabase());
    entryQuery.prepare(entryQueryString);
    entryQuery.bindValue(":displayName", QString::fromStdString(entry.displayName));
    entryQuery.bindValue(":contentHash", QString::fromStdString(entry.contentHash));
    entryQuery.bindValue(":platformId", entry.platformId);
    entryQuery.bindValue(
      ":createdAt", QVariant::fromValue(
        std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch())
        .count()));

    if (!entryQuery.exec()) {
      spdlog::error("Failed to add entry with content hash {}: {}",
                    entry.contentHash,
                    entryQuery.lastError().text().toStdString());
      return false;
    }

    entry.id = entryQuery.lastInsertId().toInt();
    return true;
  }

  QSqlDatabase SqliteUserLibraryRepository::getDatabase() const {
    const auto name =
        DATABASE_PREFIX +
        QString::number(reinterpret_cast<quint64>(QThread::currentThread()), 16);
    if (QSqlDatabase::contains(name)) {
      return QSqlDatabase::database(name);
    }

    spdlog::debug("Database connection with name {} does not exist; creating",
                  name.toStdString());
    auto db = QSqlDatabase::addDatabase("QSQLITE", name);
    db.setDatabaseName(m_databasePath);
    if (!db.open()) {
      spdlog::error("Failed to open library database connection '{}'",
                    name.toStdString());
    }
    return db;
  }
} // namespace firelight::library

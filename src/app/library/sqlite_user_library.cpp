#include "sqlite_user_library.hpp"

#include <QSqlError>
#include <QSqlQuery>
#include <QThread>
#include <qfileinfo.h>
#include <spdlog/spdlog.h>
#include <utility>

namespace firelight::library {
SqliteUserLibrary::SqliteUserLibrary(QString path, QString mainGameDirectory)
    : m_mainGameDirectory(std::move(mainGameDirectory)),
      m_databasePath(std::move(path)) {
  m_settings.beginGroup("Library");
  m_mainGameDirectory =
      m_settings.value("MainGameDirectory", m_mainGameDirectory).toString();

  QSqlQuery createRomFilesTable(getDatabase());
  createRomFilesTable.prepare("CREATE TABLE IF NOT EXISTS rom_files("
                              "id INTEGER PRIMARY KEY,"
                              "file_path TEXT NOT NULL,"
                              "file_size INTEGER NOT NULL,"
                              "file_md5 TEXT NOT NULL,"
                              "file_crc32 TEXT NOT NULL,"
                              "in_archive INTEGER NOT NULL DEFAULT 0,"
                              "archive_file_path TEXT,"
                              "platform_id INTEGER NOT NULL,"
                              "content_hash TEXT NOT NULL,"
                              "firelight_rom_id INTEGER,"
                              "created_at INTEGER NOT NULL);");

  if (!createRomFilesTable.exec()) {
    spdlog::error("Table creation failed: {}",
                  createRomFilesTable.lastError().text().toStdString());
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
      "rom_id INTEGER NOT NULL,"
      "patch_id INTEGER,"
      "created_at INTEGER NOT NULL,"
      "UNIQUE (type, rom_id, patch_id),"
      "UNIQUE (type, rom_id));");

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

  QSqlQuery createSubdirectoriesTable(getDatabase());
  createSubdirectoriesTable.prepare(
      "CREATE TABLE IF NOT EXISTS subdirectories("
      "id INTEGER PRIMARY KEY,"
      "path TEXT NOT NULL,"
      "num_files INTEGER NOT NULL DEFAULT 0,"
      "num_content_files INTEGER NOT NULL DEFAULT 0,"
      "last_modified INTEGER DEFAULT 0,"
      "recursive INTEGER NOT NULL DEFAULT 1,"
      "created_at INTEGER NOT NULL);");

  if (!createSubdirectoriesTable.exec()) {
    spdlog::error("Table creation failed: {}",
                  createSubdirectoriesTable.lastError().text().toStdString());
  }

  QSqlQuery createFoldersTable(getDatabase());
  createFoldersTable.prepare("CREATE TABLE IF NOT EXISTS folders("
                             "id INTEGER PRIMARY KEY,"
                             "display_name TEXT UNIQUE NOT NULL,"
                             "description TEXT,"
                             "icon_source_url TEXT,"
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
      "CREATE UNIQUE INDEX IF NOT EXISTS pathIdx ON rom_files(file_path);");

  if (!createRomFileUniqueIndex.exec()) {
    spdlog::error("Query failed: {}",
                  createRomFileUniqueIndex.lastError().text().toStdString());
  }

  if (!m_mainGameDirectory.isEmpty()) {
    WatchedDirectory main{.path = m_mainGameDirectory};
    create(main);
    setMainGameDirectory("");
  }

  connect(
      this, &SqliteUserLibrary::romFileAdded, this,
      [&](const int id, const QString &romPath, const int platformId,
          const QString &contentHash) {
        createRomRunConfiguration(id, romPath, platformId, contentHash);
      },
      Qt::DirectConnection);

  connect(
      this, &SqliteUserLibrary::romRunConfigurationCreated, this,
      [&](const int id, const QString &romPath, const int platformId,
          const QString &contentHash) {
        if (auto entry = getEntryWithContentHash(contentHash)) {
          if (entry->hidden) {
            entry->hidden = false;
            update(*entry);
          }
        } else {
          auto newEntry =
              Entry{.displayName = romPath.split("/").last(),
                    .contentHash = contentHash,
                    .platformId = static_cast<unsigned>(platformId)};
          createEntry(newEntry);
        }
      },
      Qt::DirectConnection);

  connect(
      this, &SqliteUserLibrary::romRunConfigurationDeleted, this,
      [&](const QString &contentHash) {
        if (const auto runConfigs = getRunConfigurations(contentHash);
            runConfigs.empty()) {
          if (auto entry = getEntryWithContentHash(contentHash)) {
            entry->hidden = true;
            update(*entry);
            emit entryHidden(entry->id);
          }
        }
      },
      Qt::DirectConnection);
}

SqliteUserLibrary::~SqliteUserLibrary() {
  m_settings.setValue("MainGameDirectory", m_mainGameDirectory);
  for (const auto &name : QSqlDatabase::connectionNames()) {
    if (name.startsWith(DATABASE_PREFIX)) {
      QSqlDatabase::removeDatabase(name);
    }
  }
}

bool SqliteUserLibrary::create(FolderInfo &folder) {
  QSqlQuery query(getDatabase());
  query.prepare("INSERT INTO folders("
                "display_name, "
                "description, "
                "icon_source_url, "
                "created_at) VALUES"
                "(:displayName, :description, :iconSourceUrl, :createdAt);");

  query.bindValue(":displayName", QString::fromStdString(folder.displayName));
  query.bindValue(":description", QString::fromStdString(folder.description));
  query.bindValue(":iconSourceUrl",
                  QString::fromStdString(folder.iconSourceUrl));
  query.bindValue(":createdAt", QDateTime::currentSecsSinceEpoch());

  if (!query.exec()) {
    spdlog::error("Failed to create folder: {}",
                  query.lastError().text().toStdString());
    return false;
  }

  folder.id = query.lastInsertId().toInt();

  return true;
}

bool SqliteUserLibrary::create(FolderEntryInfo &folderEntry) {
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

  emit entryAddedToFolder(folderEntry.folderId, folderEntry.entryId);
  return true;
}

std::vector<FolderInfo>
SqliteUserLibrary::listFolders(const FolderInfo filter) {
  // TODO: Implement filtering

  QSqlQuery query(getDatabase());
  query.prepare("SELECT * FROM folders");

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
        .createdAt = query.value("created_at").toULongLong()});
  }

  return folders;
}

bool SqliteUserLibrary::deleteFolder(int folderId) {
  QSqlQuery query(getDatabase());
  query.prepare("DELETE FROM folders WHERE id = :folderId;");
  query.bindValue(":folderId", folderId);

  if (!query.exec()) {
    spdlog::error("Failed to delete folder with ID {}: {}", folderId,
                  query.lastError().text().toStdString());
    return false;
  }

  query.finish();

  // Also delete all entries in the folder_entries table
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

bool SqliteUserLibrary::update(FolderInfo &folder) {
  if (folder.id <= 0) {
    spdlog::error("Cannot update folder with invalid ID: {}", folder.id);
    return false;
  }

  QSqlQuery query(getDatabase());
  query.prepare("UPDATE folders SET "
                "display_name = :displayName, "
                "description = :description, "
                "icon_source_url = :iconSourceUrl "
                "WHERE id = :folderId;");

  query.bindValue(":folderId", folder.id);
  query.bindValue(":displayName", QString::fromStdString(folder.displayName));
  query.bindValue(":description", QString::fromStdString(folder.description));
  query.bindValue(":iconSourceUrl",
                  QString::fromStdString(folder.iconSourceUrl));

  if (!query.exec() || query.numRowsAffected() == 0) {
    spdlog::error("Failed to update folder with ID {}: {}", folder.id,
                  query.lastError().text().toStdString());
    return false;
  }

  return true;
}

bool SqliteUserLibrary::deleteFolderEntry(FolderEntryInfo &info) {
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

bool SqliteUserLibrary::update(Entry &entry) {
  QSqlQuery query(getDatabase());
  query.prepare("UPDATE entriesv1 SET "
                "display_name = :displayName, "
                "active_save_slot = :activeSaveSlot, "
                "hidden = :hidden WHERE id = :id;");

  query.bindValue(":id", entry.id);
  query.bindValue(":displayName", entry.displayName);
  query.bindValue(":activeSaveSlot", entry.activeSaveSlot);
  query.bindValue(":hidden", entry.hidden ? 1 : 0);
  if (!query.exec() || query.numRowsAffected() == 0) {
    spdlog::error("Failed to update entry with ID {}: {}", entry.id,
                  query.lastError().text().toStdString());
    return false;
  }

  return true;
}

void SqliteUserLibrary::setMainGameDirectory(const QString &directory) {
  auto temp = directory;

  if (temp.startsWith("file://")) {
    temp = temp.remove(0, 7);
  }
  if (temp.startsWith("/")) {
    temp = temp.remove(0, 1);
  }

  if (temp == m_mainGameDirectory) {
    return;
  }

  m_mainGameDirectory = temp;
  m_settings.setValue("MainGameDirectory", m_mainGameDirectory);
  emit mainGameDirectoryChanged(m_mainGameDirectory);
}

bool SqliteUserLibrary::deleteContentDirectory(int id) {
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

  for (const auto &rom : getRomFiles()) {
    auto romPath = QString::fromStdString(rom.m_filePath);
    if (rom.m_inArchive) {
      romPath = QString::fromStdString(rom.m_archivePathName);
    }

    if (romPath.startsWith(path)) {
      auto found = false;
      auto contentPaths = getWatchedDirectories();
      for (const auto &contentPath : contentPaths) {
        if (romPath.startsWith(contentPath.path)) {
          found = true;
          break;
        }
      }

      if (!found) {
        deleteRomFile(rom.m_id);
      }
    }
  }

  emit watchedDirectoryRemoved(id, path);
  return true;
}

QString SqliteUserLibrary::getMainGameDirectory() {
  return m_mainGameDirectory;
}

bool SqliteUserLibrary::create(RomFileInfo &romFile) {
  const QString queryString = "INSERT INTO rom_files ("
                              "file_path, "
                              "file_size, "
                              "file_md5, "
                              "file_crc32,"
                              "in_archive, "
                              "archive_file_path, "
                              "platform_id, "
                              "content_hash, "
                              "created_at) VALUES"
                              "(:filePath, :fileSize, :fileMd5, :fileCrc32, "
                              ":inArchive, :archiveFilePath, :platformId, "
                              ":contentHash, :createdAt);";
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
  query.bindValue(":createdAt",
                  QVariant::fromValue(std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::system_clock::now().time_since_epoch())
                      .count()));

  if (!query.exec()) {
    spdlog::error("Failed to add rom file with path {}: {}", romFile.m_filePath,
                  query.lastError().text().toStdString());
    return false;
  }

  romFile.m_id = query.lastInsertId().toInt();

  emit romFileAdded(romFile.m_id, QString::fromStdString(romFile.m_filePath),
                    romFile.m_platformId,
                    QString::fromStdString(romFile.m_contentHash));
  return true;
}

std::optional<RomFileInfo> SqliteUserLibrary::getRomFileWithPathAndSize(
    const QString &filePath, const size_t fileSizeBytes, const bool inArchive) {
  const QString queryString =
      "SELECT * FROM rom_files WHERE file_path = :filePath AND file_size = "
      ":fileSize AND in_archive = :inArchive;";
  QSqlQuery query(getDatabase());
  query.prepare(queryString);
  query.bindValue(":filePath", filePath);
  query.bindValue(":fileSize", QVariant::fromValue(fileSizeBytes));
  query.bindValue(":inArchive", inArchive);

  if (!query.exec()) {
    spdlog::error("Failed to get rom file with path {}: {}",
                  filePath.toStdString(),
                  query.lastError().text().toStdString());
  }

  if (!query.next()) {
    return std::nullopt;
  }

  return {RomFileInfo{
      .m_id = query.value("id").toInt(),
      .m_fileSizeBytes =
          static_cast<size_t>(query.value("file_size").toLongLong()),
      .m_filePath = query.value("file_path").toString().toStdString(),
      .m_fileMd5 = query.value("file_md5").toString().toStdString(),
      .m_inArchive = query.value("in_archive").toBool(),
      .m_archivePathName =
          query.value("archive_file_path").toString().toStdString(),
      .m_platformId = query.value("platform_id").toInt(),
      .m_contentHash = query.value("content_hash").toString().toStdString(),
  }};
}
bool SqliteUserLibrary::deleteRomFile(int id) {
  QSqlQuery query(getDatabase());
  query.prepare("SELECT * FROM rom_files WHERE id = :id;");
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
  deleteQuery.prepare("DELETE FROM rom_files WHERE id = :id;");
  deleteQuery.bindValue(":id", id);

  if (!deleteQuery.exec()) {
    spdlog::error("Failed to delete rom file with ID {}: {}", id,
                  deleteQuery.lastError().text().toStdString());
    return false;
  }

  // Also delete any run configurations associated with this rom file
  QSqlQuery deleteRunConfigsQuery(getDatabase());
  deleteRunConfigsQuery.prepare("DELETE FROM run_configurations WHERE type = "
                                "\"rom\" AND rom_id = :romId;");
  deleteRunConfigsQuery.bindValue(":romId", id);

  if (!deleteRunConfigsQuery.exec()) {
    spdlog::error("Failed to delete run configurations for rom file ID {}: {} ",
                  id, deleteRunConfigsQuery.lastError().text().toStdString());
    return false;
  }

  emit romRunConfigurationDeleted(contentHash);
  emit romFileDeleted(id);
  return true;
}

std::vector<Entry> SqliteUserLibrary::getEntries(int offset, int limit) {
  const QString queryString = R"(
            SELECT e.*,
                   CASE
                       WHEN EXISTS (SELECT 1 FROM rom_files rf WHERE rf.content_hash = e.content_hash)
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
    // Get the entry ID
    int entryId = query.value("id").toInt();
    // Query for folder IDs for this entry
    // QSqlQuery folderQuery(getDatabase());
    // folderQuery.prepare("SELECT folder_id FROM folder_entries WHERE entry_id
    // = :entryId;"); folderQuery.bindValue(":entryId", entryId);
    // std::vector<int> folderIds;
    // if (folderQuery.exec()) {
    //   while (folderQuery.next()) {
    //     folderIds.push_back(folderQuery.value("folder_id").toInt());
    //   }
    // } else {
    //   spdlog::error("Failed to get folder IDs for entry {}: {}", entryId,
    //   folderQuery.lastError().text().toStdString());
    // }

    entries.emplace_back(Entry{
        .id = entryId,
        .displayName = query.value("display_name").toString(),
        .contentHash = query.value("content_hash").toString(),
        .platformId = query.value("platform_id").toUInt(),
        .activeSaveSlot = query.value("active_save_slot").toUInt(),
        // TODO
        .hidden = query.value("hidden").toBool(),
        .icon1x1SourceUrl = query.value("icon_1x1_source_url").toString(),
        .boxartFrontSourceUrl =
            query.value("boxart_front_source_url").toString(),
        .boxartBackSourceUrl = query.value("boxart_back_source_url").toString(),
        .description = query.value("description").toString(),
        .releaseYear = query.value("release_year").toUInt(),
        .developer = query.value("developer").toString(),
        .publisher = query.value("publisher").toString(),
        .genres = query.value("genres").toString(),
        .regionIds = query.value("region_ids").toString(),
        .retroachievementsSetId =
            query.value("retroachievements_set_id").toUInt(),
        .createdAt = query.value("created_at").toUInt()});
  }

  query.finish();

  for (auto &entry : entries) {
    // Get folder IDs for each entry
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
  }

  return entries;
}

std::optional<Entry> SqliteUserLibrary::getEntry(const int entryId) {
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

  auto entry = Entry{
      .id = query.value("id").toInt(),
      .displayName = query.value("display_name").toString(),
      .contentHash = query.value("content_hash").toString(),
      .platformId = query.value("platform_id").toUInt(),
      .activeSaveSlot = query.value("active_save_slot").toUInt(),
      .hidden = query.value("hidden").toBool(),
      .icon1x1SourceUrl = query.value("icon_1x1_source_url").toString(),
      .boxartFrontSourceUrl = query.value("boxart_front_source_url").toString(),
      .boxartBackSourceUrl = query.value("boxart_back_source_url").toString(),
      .description = query.value("description").toString(),
      .releaseYear = query.value("release_year").toUInt(),
      .developer = query.value("developer").toString(),
      .publisher = query.value("publisher").toString(),
      .genres = query.value("genres").toString(),
      .regionIds = query.value("region_ids").toString(),
      .retroachievementsSetId =
          query.value("retroachievements_set_id").toUInt(),
      .createdAt = query.value("created_at").toUInt()};

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

  return entry;
}

std::optional<Entry>
SqliteUserLibrary::getEntryWithContentHash(const QString &contentHash) {
  const QString queryString =
      "SELECT * FROM entriesv1 WHERE content_hash = :contentHash LIMIT 1;";
  QSqlQuery query(getDatabase());
  query.prepare(queryString);
  query.bindValue(":contentHash", contentHash);

  if (!query.exec()) {
    spdlog::error("Failed to get entry with content hash {}: {}",
                  contentHash.toStdString(),
                  query.lastError().text().toStdString());
    return {};
  }

  if (!query.next()) {
    return {};
  }

  auto entry = Entry{
      .id = query.value("id").toInt(),
      .displayName = query.value("display_name").toString(),
      .contentHash = query.value("content_hash").toString(),
      .platformId = query.value("platform_id").toUInt(),
      .activeSaveSlot = query.value("active_save_slot").toUInt(),
      .hidden = query.value("hidden").toBool(),
      .icon1x1SourceUrl = query.value("icon_1x1_source_url").toString(),
      .boxartFrontSourceUrl = query.value("boxart_front_source_url").toString(),
      .boxartBackSourceUrl = query.value("boxart_back_source_url").toString(),
      .description = query.value("description").toString(),
      .releaseYear = query.value("release_year").toUInt(),
      .developer = query.value("developer").toString(),
      .publisher = query.value("publisher").toString(),
      .genres = query.value("genres").toString(),
      .regionIds = query.value("region_ids").toString(),
      .retroachievementsSetId =
          query.value("retroachievements_set_id").toUInt(),
      .createdAt = query.value("created_at").toUInt()};

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

  return entry;
}

std::vector<RunConfiguration>
SqliteUserLibrary::getRunConfigurations(const QString &contentHash) {
  const auto queryString =
      "SELECT * FROM run_configurations WHERE content_hash = :contentHash;";
  QSqlQuery query(getDatabase());
  query.prepare(queryString);

  query.bindValue(":contentHash", contentHash);
  if (!query.exec()) {
    spdlog::error("Failed to get run configurations: {}",
                  query.lastError().text().toStdString());
  }

  std::vector<RunConfiguration> runConfigurations;
  while (query.next()) {
    RunConfiguration runConfiguration{
        .id = query.value("id").toInt(),
        .type = query.value("type").toString().toStdString(),
        .contentHash = contentHash.toStdString(),
        .romId = query.value("rom_id").toInt(),
        .patchId = query.value("patch_id").toInt(),
        .createdAt = query.value("created_at").toUInt()};

    runConfigurations.push_back(runConfiguration);
  }

  return runConfigurations;
}

void SqliteUserLibrary::doStuffWithRunConfigurations() {
  // Get all roms, make sure a run configuration exists
  // Get all patches, see if we have a match, if so, create a run configuration
  // Get all run configurations, make sure that the roms and patches exist

  auto romFiles = getRomFiles();
  for (auto &romFile : romFiles) {
    auto path =
        romFile.m_inArchive ? romFile.m_archivePathName : romFile.m_filePath;

    QFileInfo info(QString::fromStdString(path));
    if (!info.exists()) {

      continue;
    }

    createRomRunConfiguration(romFile.m_id, QString::fromStdString(path),
                              romFile.m_platformId,
                              QString::fromStdString(romFile.m_contentHash));
  }
}

std::vector<RomFileInfo>
SqliteUserLibrary::getRomFilesWithContentHash(const QString &contentHash) {
  const QString queryString =
      "SELECT * FROM rom_files WHERE content_hash = :contentHash;";
  QSqlQuery query(getDatabase());
  query.prepare(queryString);
  query.bindValue(":contentHash", contentHash);

  if (!query.exec()) {
    spdlog::error("Failed to get rom files with content hash {}: {}",
                  contentHash.toStdString(),
                  query.lastError().text().toStdString());
  }

  std::vector<RomFileInfo> romFiles;

  while (query.next()) {
    romFiles.emplace_back(RomFileInfo{
        .m_id = query.value("id").toInt(),
        .m_fileSizeBytes =
            static_cast<size_t>(query.value("file_size").toLongLong()),
        .m_filePath = query.value("file_path").toString().toStdString(),
        .m_fileMd5 = query.value("file_md5").toString().toStdString(),
        .m_inArchive = query.value("in_archive").toBool(),
        .m_archivePathName =
            query.value("archive_file_path").toString().toStdString(),
        .m_platformId = query.value("platform_id").toInt(),
        .m_contentHash = query.value("content_hash").toString().toStdString(),
    });
  }

  return romFiles;
}

std::vector<RomFileInfo> SqliteUserLibrary::getRomFiles() {
  const QString queryString = "SELECT * FROM rom_files;";
  QSqlQuery query(getDatabase());
  query.prepare(queryString);

  if (!query.exec()) {
    spdlog::error("Failed to get rom files: {}",
                  query.lastError().text().toStdString());
  }

  std::vector<RomFileInfo> romFiles;

  while (query.next()) {
    romFiles.emplace_back(RomFileInfo{
        .m_id = query.value("id").toInt(),
        .m_fileSizeBytes =
            static_cast<size_t>(query.value("file_size").toLongLong()),
        .m_filePath = query.value("file_path").toString().toStdString(),
        .m_fileMd5 = query.value("file_md5").toString().toStdString(),
        .m_inArchive = query.value("in_archive").toBool(),
        .m_archivePathName =
            query.value("archive_file_path").toString().toStdString(),
        .m_platformId = query.value("platform_id").toInt(),
        .m_contentHash = query.value("content_hash").toString().toStdString(),
    });
  }

  return romFiles;
}

std::optional<RomFileInfo> SqliteUserLibrary::getRomFile(int id) {
  auto queryString = "SELECT * FROM rom_files WHERE id = :id LIMIT 1;";
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

  return RomFileInfo{
      .m_id = query.value("id").toInt(),
      .m_fileSizeBytes =
          static_cast<size_t>(query.value("file_size").toLongLong()),
      .m_filePath = query.value("file_path").toString().toStdString(),
      .m_fileMd5 = query.value("file_md5").toString().toStdString(),
      .m_inArchive = query.value("in_archive").toBool(),
      .m_archivePathName =
          query.value("archive_file_path").toString().toStdString(),
      .m_platformId = query.value("platform_id").toInt(),
      .m_contentHash = query.value("content_hash").toString().toStdString(),
  };
}
std::optional<PatchFile> SqliteUserLibrary::getPatchFile(int id) {
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

void SqliteUserLibrary::create(PatchFile &file) {
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
                  QVariant::fromValue(std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::system_clock::now().time_since_epoch())
                      .count()));

  if (!query.exec()) {
    spdlog::error("Failed to add patch file with path {}: {}", file.m_filePath,
                  query.lastError().text().toStdString());
  }
}

std::optional<PatchFile> SqliteUserLibrary::getPatchFileWithPathAndSize(
    const QString &filePath, size_t fileSizeBytes, bool inArchive) {}

std::vector<PatchFile> SqliteUserLibrary::getPatchFiles() {}

std::vector<WatchedDirectory> SqliteUserLibrary::getWatchedDirectories() {
  const QString queryString = "SELECT * FROM content_directoriesv1;";
  QSqlQuery query(getDatabase());
  query.prepare(queryString);

  if (!query.exec()) {
    spdlog::error("Failed to get directories: {}",
                  query.lastError().text().toStdString());
  }

  std::vector<WatchedDirectory> directories;

  while (query.next()) {
    directories.emplace_back(WatchedDirectory{
        .id = query.value("id").toInt(),
        .path = query.value("path").toString(),
        .numFiles = query.value("num_files").toInt(),
        .numContentFiles = query.value("num_content_files").toInt(),
        .lastModified = QDateTime::fromMSecsSinceEpoch(
            query.value("last_modified").toUInt()),
        .recursive = query.value("recursive").toBool(),
        .createdAt = query.value("created_at").toUInt()});
  }

  return directories;
}

bool SqliteUserLibrary::create(WatchedDirectory &directory) {
  const QString queryString = "INSERT OR IGNORE INTO content_directoriesv1 ("
                              "path, "
                              "created_at) VALUES"
                              "(:path, :createdAt);";
  QSqlQuery query(getDatabase());
  query.prepare(queryString);
  query.bindValue(":path", directory.path);
  query.bindValue(":createdAt",
                  QVariant::fromValue(std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::system_clock::now().time_since_epoch())
                      .count()));

  if (!query.exec()) {
    spdlog::error("Failed to add directory with path {}: {}",
                  directory.path.toStdString(),
                  query.lastError().text().toStdString());
    return false;
  }

  if (query.numRowsAffected() == 0) {
    return false;
  }

  directory.id = query.lastInsertId().toInt();
  emit watchedDirectoryAdded(directory.id, directory.path);
  return true;
}

bool SqliteUserLibrary::update(const WatchedDirectory &directory) {
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
  q.bindValue(":path", directory.path);
  q.bindValue(":id", directory.id);

  if (!q.exec()) {
    spdlog::error("Failed to update watched directory: {}",
                  q.lastError().text().toStdString());
    return false;
  }

  emit watchedDirectoryUpdated(directory.id, oldPath, directory.path);
  return true;
}

void SqliteUserLibrary::createRomRunConfiguration(const int romId,
                                                  const QString &path,
                                                  const int platformId,
                                                  const QString &contentHash) {
  const QString insertQueryString =
      "INSERT OR IGNORE INTO run_configurations "
      "(type, content_hash, rom_id, created_at) "
      "VALUES (:type, :contentHash, :romId, :createdAt);";

  QSqlQuery query(getDatabase());
  query.prepare(insertQueryString);

  query.bindValue(":type", "rom");
  query.bindValue(":contentHash", contentHash);
  query.bindValue(":romId", romId);
  query.bindValue(":createdAt",
                  QDateTime::currentSecsSinceEpoch()); // Current timestamp

  if (!query.exec()) {
    spdlog::error("Failed to create run configuration: {}",
                  query.lastError().text().toStdString());
  }

  emit romRunConfigurationCreated(query.lastInsertId().toInt(), path,
                                  platformId, contentHash);
}

bool SqliteUserLibrary::createEntry(Entry &entry) {
  QSqlQuery selectQuery(getDatabase());
  selectQuery.prepare(
      "SELECT * FROM entriesv1 WHERE content_hash = :contentHash;");
  selectQuery.bindValue(":contentHash", entry.contentHash);
  if (!selectQuery.exec()) {
    spdlog::error("Failed to check for existing entry with content hash {}: {}",
                  entry.contentHash.toStdString(),
                  selectQuery.lastError().text().toStdString());
    return false;
  }

  if (selectQuery.next()) {
    spdlog::debug(
        "Entry with content hash {} already exists, skipping creation",
        entry.contentHash.toStdString());

    // const QString updateQueryString = "UPDATE entriesv1 SET "
    //                                   "hidden = :hidden WHERE id = :id;";
    // QSqlQuery updateQuery(getDatabase());
    // updateQuery.prepare(updateQueryString);
    // updateQuery.bindValue(":hidden", false);
    //
    // if (!updateQuery.exec()) {
    //   spdlog::error("Failed to add entry with content hash {}: {}",
    //                 entry.contentHash.toStdString(),
    //                 updateQuery.lastError().text().toStdString());
    //   return false;
    // }

    // entry.hidden = false;
    // selectQuery.finish();
    // update(entry);

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
  entryQuery.bindValue(":displayName", entry.displayName);
  entryQuery.bindValue(":contentHash", entry.contentHash);
  entryQuery.bindValue(":platformId", entry.platformId);
  entryQuery.bindValue(":createdAt",
                       QVariant::fromValue(std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count()));

  if (!entryQuery.exec()) {
    spdlog::error("Failed to add entry with content hash {}: {}",
                  entry.contentHash.toStdString(),
                  entryQuery.lastError().text().toStdString());
    return false;
  }

  entry.id = entryQuery.lastInsertId().toInt();
  emit entryCreated(entryQuery.lastInsertId().toInt(), entry.contentHash);
  return true;
}

QSqlDatabase SqliteUserLibrary::getDatabase() const {
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
  db.open();
  return db;
}
} // namespace firelight::library

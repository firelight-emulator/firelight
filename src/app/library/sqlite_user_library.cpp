#include "sqlite_user_library.hpp"

#include <QThread>
#include <QSqlQuery>
#include <QSqlError>
#include <utility>
#include <spdlog/spdlog.h>

namespace firelight::library {
    SqliteUserLibrary::SqliteUserLibrary(QString path) : m_databasePath(std::move(path)) {
        QSqlQuery createRomFilesTable(getDatabase());
        createRomFilesTable.prepare("CREATE TABLE IF NOT EXISTS rom_files("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
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

        QSqlQuery createEntriesTable(getDatabase());
        createEntriesTable.prepare("CREATE TABLE IF NOT EXISTS entriesv1("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
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
            "created_at INTEGER NOT NULL);");

        if (!createEntriesTable.exec()) {
            spdlog::error("Table creation failed: {}",
                          createEntriesTable.lastError().text().toStdString());
        }

        QSqlQuery createDirectoriesTable(getDatabase());
        createDirectoriesTable.prepare("CREATE TABLE IF NOT EXISTS watched_directoriesv1("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "path TEXT NOT NULL,"
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
        createSubdirectoriesTable.prepare("CREATE TABLE IF NOT EXISTS subdirectories("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
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
    }

    SqliteUserLibrary::~SqliteUserLibrary() {
        for (const auto &name: QSqlDatabase::connectionNames()) {
            if (name.startsWith(DATABASE_PREFIX)) {
                QSqlDatabase::removeDatabase(name);
            }
        }
    }

    void SqliteUserLibrary::setMainGameDirectory(const QString &directory) {
        m_mainGameDirectory = directory;

        if (m_mainGameDirectory.startsWith("file://")) {
            m_mainGameDirectory = m_mainGameDirectory.remove(0, 7);
        }
        if (m_mainGameDirectory.startsWith("/")) {
            m_mainGameDirectory = m_mainGameDirectory.remove(0, 1);
        }
    }

    QString SqliteUserLibrary::getMainGameDirectory() {
        return m_mainGameDirectory;
    }

    void SqliteUserLibrary::addRomFile(RomFile &romFile) {
        const QString queryString =
                "INSERT INTO rom_files ("
                "file_path, "
                "file_size, "
                "file_md5, "
                "file_crc32,"
                "in_archive, "
                "archive_file_path, "
                "platform_id, "
                "content_hash, "
                "created_at) VALUES"
                "(:filePath, :fileSize, :fileMd5, :fileCrc32, :inArchive, :archiveFilePath, :platformId, "
                ":contentHash, :createdAt);";
        QSqlQuery query(getDatabase());
        query.prepare(queryString);
        query.bindValue(":filePath", romFile.getFilePath());
        query.bindValue(":fileSize", romFile.getFileSizeBytes());
        query.bindValue(":fileMd5", romFile.getFileMd5());
        query.bindValue(":fileCrc32", romFile.getFileCrc32());
        query.bindValue(":inArchive", romFile.inArchive());
        query.bindValue(":archiveFilePath", romFile.getArchivePathName());
        query.bindValue(":platformId", romFile.getPlatformId());
        query.bindValue(":contentHash", romFile.getContentHash());
        query.bindValue(":createdAt",
                        std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                        .count());

        if (!query.exec()) {
            spdlog::error("Failed to add rom file with path {}: {}", romFile.getFilePath().toStdString(),
                          query.lastError().text().toStdString());
            return;
        }

        emit romFileAdded(query.lastInsertId().toInt(), romFile.getContentHash());

        const QString getEntryQueryString =
                "SELECT id FROM entriesv1 WHERE content_hash = :contentHash;";
        QSqlQuery getEntryQuery(getDatabase());
        getEntryQuery.prepare(getEntryQueryString);
        getEntryQuery.bindValue(":contentHash", romFile.getContentHash());

        if (!getEntryQuery.exec()) {
            spdlog::error("Failed to get entry with content hash {}: {}", romFile.getContentHash().toStdString(),
                          getEntryQuery.lastError().text().toStdString());
            return;
        }

        if (!getEntryQuery.next()) {
            const auto filename = romFile.getFilePath().split("/").last();

            const QString entryQueryString =
                    "INSERT INTO entriesv1 ("
                    "display_name, "
                    "content_hash, "
                    "platform_id, "
                    "created_at) VALUES"
                    "(:displayName, :contentHash, :platformId, :createdAt);";
            QSqlQuery entryQuery(getDatabase());
            entryQuery.prepare(entryQueryString);
            entryQuery.bindValue(":displayName", filename);
            entryQuery.bindValue(":contentHash", romFile.getContentHash());
            entryQuery.bindValue(":platformId", romFile.getPlatformId());
            entryQuery.bindValue(":createdAt",
                                 std::chrono::duration_cast<std::chrono::milliseconds>(
                                     std::chrono::system_clock::now().time_since_epoch())
                                 .count());

            if (!entryQuery.exec()) {
                spdlog::error("Failed to add entry with content hash {}: {}", romFile.getContentHash().toStdString(),
                              query.lastError().text().toStdString());
                return;
            }

            emit entryCreated(entryQuery.lastInsertId().toInt(), romFile.getContentHash());
        }
    }

    std::optional<RomFile> SqliteUserLibrary::getRomFileWithPathAndSize(const QString &filePath,
                                                                        const size_t fileSizeBytes,
                                                                        const bool inArchive) {
        const QString queryString =
                "SELECT * FROM rom_files WHERE file_path = :filePath AND file_size = :fileSize AND in_archive = :inArchive;";
        QSqlQuery query(getDatabase());
        query.prepare(queryString);
        query.bindValue(":filePath", filePath);
        query.bindValue(":fileSize", fileSizeBytes);
        query.bindValue(":inArchive", inArchive);

        if (!query.exec()) {
            spdlog::error("Failed to get rom file with path {}: {}", filePath.toStdString(),
                          query.lastError().text().toStdString());
        }

        if (!query.next()) {
            return std::nullopt;
        }

        return {RomFile(query)};
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
            spdlog::error("Failed to get entries: {}", query.lastError().text().toStdString());
            return {};
        }

        std::vector<Entry> entries;

        while (query.next()) {
            entries.emplace_back(Entry{
                .id = query.value("id").toInt(),
                .displayName = query.value("display_name").toString(),
                .contentHash = query.value("content_hash").toString(),
                .platformId = query.value("platform_id").toUInt(),
                .activeSaveSlot = query.value("active_save_slot").toUInt(),
                // TODO

                // .hidden = query.value("hidden").toBool(),
                .hidden = !query.value("has_rom").toBool(),
                .icon1x1SourceUrl = query.value("icon_1x1_source_url").toString(),
                .boxartFrontSourceUrl = query.value("boxart_front_source_url").toString(),
                .boxartBackSourceUrl = query.value("boxart_back_source_url").toString(),
                .description = query.value("description").toString(),
                .releaseYear = query.value("release_year").toUInt(),
                .developer = query.value("developer").toString(),
                .publisher = query.value("publisher").toString(),
                .genres = query.value("genres").toString(),
                .regionIds = query.value("region_ids").toString(),
                .createdAt = query.value("created_at").toUInt()
            });
        }

        return entries;
    }

    std::optional<Entry> SqliteUserLibrary::getEntry(const int entryId) {
        const QString queryString = "SELECT * FROM entriesv1 WHERE id = :entryId LIMIT 1;";
        QSqlQuery query(getDatabase());
        query.prepare(queryString);
        query.bindValue(":entryId", entryId);

        if (!query.exec()) {
            spdlog::error("Failed to get entry: {}", query.lastError().text().toStdString());
            return {};
        }

        if (!query.next()) {
            spdlog::error("Failed to get entry: {}", query.lastError().text().toStdString());
            return {};
        }

        return {
            Entry{
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
                .createdAt = query.value("created_at").toUInt()
            }
        };
    }

    std::vector<RomFile> SqliteUserLibrary::getRomFilesWithContentHash(const QString &contentHash) {
        const QString queryString =
                "SELECT * FROM rom_files WHERE content_hash = :contentHash;";
        QSqlQuery query(getDatabase());
        query.prepare(queryString);
        query.bindValue(":contentHash", contentHash);

        if (!query.exec()) {
            spdlog::error("Failed to get rom files with content hash {}: {}", contentHash.toStdString(),
                          query.lastError().text().toStdString());
        }

        std::vector<RomFile> romFiles;

        while (query.next()) {
            romFiles.emplace_back(query);
        }

        return romFiles;
    }

    std::vector<RomFile> SqliteUserLibrary::getRomFiles() {
        const QString queryString =
                "SELECT * FROM rom_files;";
        QSqlQuery query(getDatabase());
        query.prepare(queryString);

        if (!query.exec()) {
            spdlog::error("Failed to get rom files: {}", query.lastError().text().toStdString());
        }

        std::vector<RomFile> directories;

        while (query.next()) {
            directories.emplace_back(query);
        }

        return directories;
    }

    bool SqliteUserLibrary::removeRomFile(const QString &filePath, bool inArchive, const QString &archivePath) {
        if (inArchive) {
            QSqlQuery query(getDatabase());
            query.prepare(
                "DELETE FROM rom_files WHERE file_path = :filePath AND in_archive = 1 AND archive_file_path = :archivePath;");
            query.bindValue(":filePath", filePath);
            query.bindValue(":archivePath", archivePath);

            if (!query.exec()) {
                spdlog::error("Failed to delete rom file: {}",
                              query.lastError().text().toStdString());
                return false;
            }

            return true;
        }

        QSqlQuery query(getDatabase());
        query.prepare("DELETE FROM rom_files WHERE file_path = :filePath AND in_archive = 0;");
        query.bindValue(":filePath", filePath);

        if (!query.exec()) {
            spdlog::error("Failed to delete rom file: {}",
                          query.lastError().text().toStdString());
            return false;
        }

        return true;
    }

    std::vector<WatchedDirectory> SqliteUserLibrary::getWatchedDirectories() {
        const QString queryString =
                "SELECT * FROM watched_directoriesv1;";
        QSqlQuery query(getDatabase());
        query.prepare(queryString);

        if (!query.exec()) {
            spdlog::error("Failed to get directories: {}", query.lastError().text().toStdString());
        }

        std::vector<WatchedDirectory> directories;

        while (query.next()) {
            directories.emplace_back(WatchedDirectory{
                .id = query.value("id").toInt(),
                .path = query.value("path").toString(),
                .numFiles = query.value("num_files").toInt(),
                .numContentFiles = query.value("num_content_files").toInt(),
                .lastModified = QDateTime::fromMSecsSinceEpoch(query.value("last_modified").toUInt()),
                .recursive = query.value("recursive").toBool(),
                .createdAt = query.value("created_at").toUInt()
            });
        }

        return directories;
    }

    bool SqliteUserLibrary::addWatchedDirectory(const WatchedDirectory &directory) {
        const QString queryString =
                "INSERT INTO watched_directoriesv1 ("
                "path, "
                "created_at) VALUES"
                "(:path, :createdAt);";
        QSqlQuery query(getDatabase());
        query.prepare(queryString);
        query.bindValue(":path", directory.path);
        query.bindValue(":createdAt",
                        std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                        .count());

        if (!query.exec()) {
            spdlog::error("Failed to add directory with path {}: {}", directory.path.toStdString(),
                          query.lastError().text().toStdString());
            return false;
        }

        emit watchedDirectoryAdded(query.lastInsertId().toInt(), directory.path);
        return true;
    }

    bool SqliteUserLibrary::updateWatchedDirectory(const WatchedDirectory &directory) {
        QSqlQuery q(getDatabase());
        q.prepare("UPDATE watched_directoriesv1 SET path = :path WHERE id = :id");
        q.bindValue(":path", directory.path);
        q.bindValue(":id", directory.id);

        if (!q.exec()) {
            spdlog::error("Failed to update watched directory: {}",
                          q.lastError().text().toStdString());
            return false;
        }

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
} // library

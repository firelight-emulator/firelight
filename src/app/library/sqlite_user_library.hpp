#pragma once
#include <QSqlDatabase>
#include <QString>
#include <firelight/library/user_library.hpp>
#include <firelight/library/rom_file.hpp>

namespace firelight::library {
    class SqliteUserLibrary : public QObject, public IUserLibrary {
        Q_OBJECT

    public:
        explicit SqliteUserLibrary(QString path);

        ~SqliteUserLibrary() override;

        void addRomFile(RomFile &romFile) override;

        std::optional<RomFile> getRomFileWithPathAndSize(const QString &filePath, size_t fileSizeBytes,
                                                         bool inArchive) override;

        std::vector<Entry> getEntries(int offset, int limit) override;

        std::optional<Entry> getEntry(int entryId) override;

        std::vector<RomFile> getRomFilesWithContentHash(const QString &contentHash) override;

        std::vector<RomFile> getRomFiles() override;

        bool removeRomFile(const QString &filePath, bool inArchive, const QString &archivePath) override;

        std::vector<WatchedDirectory> getWatchedDirectories() override;

        bool addWatchedDirectory(const WatchedDirectory &directory) override;

        bool updateWatchedDirectory(const WatchedDirectory &directory) override;

    signals:
        void romFileAdded(int id, const QString &contentHash);

        void entryCreated(int id, const QString &contentHash);

        void watchedDirectoryAdded(int id, const QString &path);

    private:
        static constexpr auto DATABASE_PREFIX = "userlibrary_";

        [[nodiscard]] QSqlDatabase getDatabase() const;

        QString m_databasePath;
    };
} // library

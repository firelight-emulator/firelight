#pragma once
#include <QSqlDatabase>
#include <QString>
#include "rom_file.hpp"
#include "user_library.hpp"
#include <qsettings.h>

namespace firelight::library {
    class SqliteUserLibrary final : public QObject, public IUserLibrary {
        Q_OBJECT
        Q_PROPERTY(
            QString mainGameDirectory READ getMainGameDirectory WRITE setMainGameDirectory NOTIFY
            mainGameDirectoryChanged)

    public:
        explicit SqliteUserLibrary(QString path, QString mainGameDirectory);

        ~SqliteUserLibrary() override;

        void setMainGameDirectory(const QString &directory);

        QString getMainGameDirectory();

        void addRomFile(RomFile &romFile) override;

        std::optional<RomFile> getRomFileWithPathAndSize(const QString &filePath, size_t fileSizeBytes,
                                                         bool inArchive) override;

        std::vector<Entry> getEntries(int offset, int limit) override;

        std::optional<Entry> getEntry(int entryId) override;

        std::vector<RomFile> getRomFilesWithContentHash(const QString &contentHash) override;

        std::vector<RomFile> getRomFiles() override;

      bool removeRomFile(const QString &filePath, bool inArchive, const QString &archivePath) override;

      void addPatchFile(PatchFile &file);

      std::optional<PatchFile> getPatchFileWithPathAndSize(const QString &filePath, size_t fileSizeBytes, bool inArchive);

      std::vector<PatchFile> getPatchFiles();

      bool removePatchFile(const QString &filePath, bool inArchive, const QString &archivePath);

        std::vector<WatchedDirectory> getWatchedDirectories() override;

        bool addWatchedDirectory(const WatchedDirectory &directory) override;

        bool updateWatchedDirectory(const WatchedDirectory &directory) override;

    signals:
        void mainGameDirectoryChanged(const QString &newDirectory);

        void romFileAdded(int id, const QString &contentHash);

        void entryCreated(int id, const QString &contentHash);

        void watchedDirectoryAdded(int id, const QString &path);

    private:
        static constexpr auto DATABASE_PREFIX = "userlibrary_";

        QString m_mainGameDirectory;

        QSettings m_settings;

        [[nodiscard]] QSqlDatabase getDatabase() const;

        QString m_databasePath;
    };
} // library

#pragma once

#include <QSqlQuery>

#include "patch_file.hpp"

namespace firelight::library {
    class RomFile {
    public:
        explicit RomFile(const QString &path);

        RomFile(const QString &filename, const char *data, size_t size, const QString &archivePathName = "");

        explicit RomFile(const QSqlQuery &query);

        void load();

        bool isValid() const;

        QByteArray getContentBytes();

        QString getContentHash();

        QString getFileMd5();

        QString getFileCrc32();

        QString getFilePath();

        size_t getFileSizeBytes() const;

        bool inArchive() const;

        QString getArchivePathName();

        [[nodiscard]] int getPlatformId() const;

        RomFile applyPatch(const PatchFile &patchFile);

    private:
        void generateContentBytesAndHash(const QByteArray &fileBytes);

        int m_id = -1;
        int m_entryId = -1;
        QString m_fileCrc32 = ":)";
        size_t m_fileSizeBytes;
        QString m_filePath;
        QString m_suffix;
        int m_platformId;
        QByteArray m_contentBytes;
        QString m_contentHash;
        QString m_fileMd5;
        QString m_archivePathName;
        bool m_inArchive = false;
        bool m_isValid = false;
    };
}

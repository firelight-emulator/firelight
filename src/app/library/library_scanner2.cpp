#include "library_scanner2.hpp"
#include <qdiriterator.h>
#include <firelight/library/entry.hpp>
#include <firelight/library/rom_entry.hpp>

#include "../platform_metadata.hpp"


namespace firelight::library {
    LibraryScanner2::LibraryScanner2() {
        m_watcher.addPath(R"(C:\Users\alexs\AppData\Roaming\Firelight\roms)");
    }

    QFuture<bool> LibraryScanner2::scanAllPaths() {
        auto allDirs = m_watcher.directories();

        for (const auto &dir: allDirs) {
            QDirIterator iter(dir, QDirIterator::Subdirectories);
            while (iter.hasNext()) {
                // TODO: Check if we should stop scanning

                const auto fileInfo = iter.nextFileInfo();
                if (fileInfo.isDir()) {
                    continue;
                }

                // TODO: Check for patch formats
                const auto platformId = PlatformMetadata::getIdFromFileExtension(
                    fileInfo.suffix().toStdString());
                if (platformId == PlatformMetadata::PLATFORM_ID_UNKNOWN) {
                    continue;
                }

                // Check if we have an entry matching the path
                // If we do, check if it's the same as the one we know about

                QFile file(fileInfo.filePath());
                if (!file.open(QIODeviceBase::ReadOnly)) {
                    // TODO: error :(
                    continue;
                }

                auto fileData = file.readAll();
                file.close();

                auto md5 = QCryptographicHash::hash(fileData, QCryptographicHash::Algorithm::Md5);
                const auto contentId = calculateContentId(platformId, md5, fileData);

                // Check if we have an entry matching the md5/contentId
                // If we do, mark as duplicate?

                RomEntry romEntry;
                romEntry.displayName = fileInfo.fileName().toStdString();
                romEntry.contentId = contentId.toHex();
                romEntry.fileMd5 = md5.toHex();
                romEntry.filePath = fileInfo.filePath().toStdString();

                printf("Adding entry: %s (content ID: %s)\n", romEntry.displayName.c_str(), romEntry.contentId.c_str());
            }
        }
        return {};

        // Check for files that ARE in the database but not in the filesystem
        // Calculate changeset between all directories - for example if removed from one but added to another
    }

    QByteArray LibraryScanner2::calculateContentId(const int platformId, QByteArray &originalMd5,
                                                   QByteArray &fileData) {
        if (platformId == PlatformMetadata::PLATFORM_ID_NES) {
            if (fileData.startsWith("NES\x1A")) {
                fileData.remove(0, 16);
                return QCryptographicHash::hash(fileData, QCryptographicHash::Algorithm::Md5);
            }
        }

        if (platformId == PlatformMetadata::PLATFORM_ID_SNES) {
            if (fileData.size() % 1024 == 512) {
                fileData.remove(0, 512);
                return QCryptographicHash::hash(fileData, QCryptographicHash::Algorithm::Md5);
            }
        }

        return originalMd5;
    }
} // namespace firelight::library

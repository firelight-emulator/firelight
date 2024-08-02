#include "library_scanner2.hpp"
#include <qdiriterator.h>
#include <QtConcurrent>
#include <firelight/library/entry.hpp>
#include <firelight/library/rom_entry.hpp>

#include "../platform_metadata.hpp"


namespace firelight::library {
    LibraryScanner2::LibraryScanner2() {
        m_threadPool.setMaxThreadCount(1);
        m_watcher.addPath(R"(C:\Users\alexs\AppData\Roaming\Firelight\roms)");
        m_watcher.addPath(R"(C:\Users\alexs\AppData\Roaming\Firelight\patches)");

        connect(&m_watcher, &QFileSystemWatcher::directoryChanged,
                [&](const QString &path) {
                    queueScan(path);
                    startScan();
                });
    }

    LibraryScanner2::~LibraryScanner2() {
        m_shuttingDown = true;
    }

    QFuture<bool> LibraryScanner2::startScan() {
        if (m_scanRunning) {
            return QtConcurrent::run([] { return false; });
        }

        return QtConcurrent::run(&m_threadPool, [this] {
                                     m_scanRunning = true;
                                     m_pathQueueLock.lockForRead();
                                     auto queueEmpty = m_pathQueue.isEmpty();
                                     QString next;
                                     if (!queueEmpty) {
                                         next = m_pathQueue.dequeue();
                                         m_scanQueuedByPath[next] = false;
                                     }
                                     m_pathQueueLock.unlock();

                                     while (!queueEmpty) {
                                         printf("Scanning %s\n", next.toStdString().c_str());
                                         QDirIterator iter(next, QDirIterator::Subdirectories);
                                         while (iter.hasNext()) {
                                             if (m_shuttingDown) {
                                                 return false;
                                             }

                                             if (m_scanQueuedByPath[next]) {
                                                 printf("Stopping scan of %s\n", next.toStdString().c_str());
                                                 break;
                                             }
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

                                             auto md5 = QCryptographicHash::hash(
                                                 fileData, QCryptographicHash::Algorithm::Md5);
                                             const auto contentId = calculateContentId(platformId, md5, fileData);

                                             // Check if we have an entry matching the md5/contentId
                                             // If we do, mark as duplicate?

                                             RomEntry romEntry;
                                             romEntry.displayName = fileInfo.fileName().toStdString();
                                             romEntry.contentId = contentId.toHex();
                                             romEntry.fileMd5 = md5.toHex();
                                             romEntry.filePath = fileInfo.filePath().toStdString();

                                             // TODO: Get metadata from the database

                                             printf("Adding entry: %s (content ID: %s)\n", romEntry.displayName.c_str(),
                                                    romEntry.contentId.c_str());

                                             QThread::sleep(2);
                                         }

                                         m_pathQueueLock.lockForRead();
                                         queueEmpty = m_pathQueue.isEmpty();
                                         if (!queueEmpty) {
                                             next = m_pathQueue.dequeue();
                                             m_scanQueuedByPath[next] = false;
                                         }
                                         m_pathQueueLock.unlock();
                                     }

                                     m_scanRunning = false;
                                     return true;
                                 }

        );

        // Check for files that ARE in the database but not in the filesystem
        // Calculate changeset between all directories - for example if removed from one but added to another
    }

    void LibraryScanner2::queueScan(const QString &path) {
        m_pathQueueLock.lockForWrite();
        m_scanQueuedByPath[path] = true;
        m_pathQueue.enqueue(path);
        m_pathQueueLock.unlock();
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

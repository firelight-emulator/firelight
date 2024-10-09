#include "library_scanner2.hpp"
#include <qdiriterator.h>
#include <QtConcurrent>
#include <archive.h>
#include <archive_entry.h>
#include <firelight/library/rom_file.hpp>
#include <rcheevos/rc_hash.h>
#include <spdlog/spdlog.h>

#include "../platform_metadata.hpp"


namespace firelight::library {
    LibraryScanner2::LibraryScanner2(IUserLibrary &library) : m_library(library) {
        m_threadPool.setMaxThreadCount(1);
        for (const auto &dir: m_library.getWatchedDirectories()) {
            watchPath(dir.path);
        }

        connect(&m_watcher, &QFileSystemWatcher::directoryChanged,
                [&](const QString &path) {
                    queueScan(path);
                    startScan();
                });
    }

    LibraryScanner2::~LibraryScanner2() {
        m_shuttingDown = true;
    }

    void LibraryScanner2::watchPath(const QString &path) {
        m_watcher.addPath(path);
    }

    QFuture<bool> LibraryScanner2::startScan() {
        if (m_scanRunning) {
            return QtConcurrent::run([] { return false; });
        }

        return QtConcurrent::run(&m_threadPool, [this] {
                                     QThread::currentThread()->setPriority(QThread::NormalPriority);
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
                                         QDirIterator iter(next, QDirIterator::Subdirectories);

                                         auto dirInfo = QFileInfo(next);
                                         spdlog::info("Scanning directory: {} (last modified: {})",
                                                      dirInfo.filePath().toStdString(),
                                                      dirInfo.lastModified().toString().toStdString());

                                         // get directory info
                                         // if last modified is the same, skip

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

                                             auto extension = fileInfo.suffix().toLower();
                                             if (extension == "zip" || extension == "7z" || extension == "tar" ||
                                                 extension == "rar") {
                                                 archive_entry *entry;

                                                 archive *a = archive_read_new();
                                                 archive_read_support_filter_all(a);
                                                 archive_read_support_format_all(a);

                                                 int r = archive_read_open_filename(
                                                     a, fileInfo.filePath().toStdString().c_str(), 10240);

                                                 if (r != ARCHIVE_OK) {
                                                     continue;
                                                 }

                                                 while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
                                                     auto size = archive_entry_size(entry);
                                                     if (size > 0) {
                                                         if (m_library.getRomFileWithPathAndSize(
                                                             archive_entry_pathname(entry),
                                                             size, true)) {
                                                             printf("Already got this one; skipping\n");
                                                             continue;
                                                         }

                                                         auto buffer = new char[size];
                                                         archive_read_data(a, buffer, size);
                                                         auto romFile = RomFile(
                                                             archive_entry_pathname(entry), buffer, size,
                                                             fileInfo.filePath());

                                                         // printf("ROM (platform %d) file: %s\n", romFile.getPlatformId(),
                                                         //        fileInfo.filePath().toStdString().c_str());
                                                         // printf("-- content hash: %s\n",
                                                         //        romFile.getContentHash().toStdString().c_str());

                                                         m_library.addRomFile(romFile);

                                                         delete[] buffer;
                                                     }
                                                     archive_read_data_skip(a);
                                                 }
                                                 r = archive_read_free(a);
                                                 if (r != ARCHIVE_OK) {
                                                     continue;
                                                 }
                                                 continue;
                                             }

                                             if (extension == "ips" || extension == "bps" || extension == "ups" ||
                                                 extension == "mod") {
                                                 printf("Patch file: %s\n",
                                                        fileInfo.filePath().toStdString().c_str());
                                                 continue;
                                                 // Get md5 of the file
                                                 // Do we recognize the md5? If so, we know what the md5 (contentID?) of the target rom file is
                                                 // If we don't recognize the md5, there's nothing we can do
                                             }

                                             if (m_library.getRomFileWithPathAndSize(fileInfo.filePath(),
                                                 fileInfo.size(), false)) {
                                                 printf("Already got this one; skipping\n");
                                                 continue;
                                             }

                                             auto romFile = RomFile(fileInfo.filePath());
                                             m_library.addRomFile(romFile);

                                             // metadata scan:
                                             // for each entry

                                             // look up firelight content id using content hash
                                             // that gets us release year, description... etc.

                                             // then look up at retroachievements using content hash
                                             // to do that we need a db table with the mapping
                                             // that gets us image
                                         }

                                         // update directory info

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
        if (m_scanQueuedByPath.contains(path) && m_scanQueuedByPath[path]) {
            m_pathQueueLock.unlock();
            return;
        }

        m_scanQueuedByPath[path] = true;
        m_pathQueue.enqueue(path);
        m_pathQueueLock.unlock();
    }
} // namespace firelight::library

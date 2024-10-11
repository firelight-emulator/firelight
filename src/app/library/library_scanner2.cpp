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
                                     QString nextDirectory;
                                     if (!queueEmpty) {
                                         nextDirectory = m_pathQueue.dequeue();
                                         m_scanQueuedByPath[nextDirectory] = false;
                                     }
                                     m_pathQueueLock.unlock();

                                     QSet<QString> knownFiles;
                                     for (auto &romFile: m_library.getRomFiles()) {
                                         if (romFile.inArchive()) {
                                             knownFiles.insert(
                                                 romFile.getArchivePathName() + "|" + romFile.getFilePath());
                                         } else {
                                             knownFiles.insert(romFile.getFilePath());
                                         }
                                     }

                                     spdlog::info("Known files: {}", knownFiles.size());

                                     QSet<QString> scannedFiles;

                                     QStack<QString> subdirectories;
                                     while (!queueEmpty) {
                                         QDirIterator iter(nextDirectory,
                                                           QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

                                         auto dirInfo = QFileInfo(nextDirectory);
                                         spdlog::info("Scanning directory: {} (last modified: {})",
                                                      dirInfo.filePath().toStdString(),
                                                      dirInfo.lastModified().toString().toStdString());

                                         // get directory info
                                         // if last modified is the same, skip


                                         while (iter.hasNext()) {
                                             if (m_shuttingDown) {
                                                 return false;
                                             }

                                             if (m_scanQueuedByPath[nextDirectory]) {
                                                 printf("Stopping scan of %s\n", nextDirectory.toStdString().c_str());
                                                 break;
                                             }
                                             // TODO: Check if we should stop scanning

                                             const auto fileInfo = iter.nextFileInfo();
                                             if (fileInfo.isDir()) {
                                                 subdirectories.push(fileInfo.filePath());
                                                 continue;
                                             }

                                             scannedFiles.insert(fileInfo.filePath());

                                             if (fileInfo.size() > 1024 * 1024 * 1024) {
                                                 spdlog::info("Skipping large file: {}",
                                                              fileInfo.filePath().toStdString());
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
                                                         auto entryPathName = archive_entry_pathname(entry);
                                                         scannedFiles.insert(fileInfo.filePath() + "|" + entryPathName);
                                                         if (m_library.getRomFileWithPathAndSize(
                                                             entryPathName,
                                                             size, true)) {
                                                             spdlog::info("Skipping known file: {}",
                                                                          archive_entry_pathname(entry));
                                                             continue;
                                                         }

                                                         auto buffer = new char[size];
                                                         archive_read_data(a, buffer, size);
                                                         auto romFile = RomFile(
                                                             entryPathName, buffer, size,
                                                             fileInfo.filePath());

                                                         // printf("ROM (platform %d) file: %s\n", romFile.getPlatformId(),
                                                         //        fileInfo.filePath().toStdString().c_str());
                                                         // printf("-- content hash: %s\n",
                                                         //        romFile.getContentHash().toStdString().c_str());

                                                         if (romFile.isValid()) {
                                                             m_library.addRomFile(romFile);
                                                         }

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
                                                 spdlog::info("Skipping patch file for now: {}",
                                                              fileInfo.filePath().toStdString());
                                                 continue;
                                                 // Get md5 of the file
                                                 // Do we recognize the md5? If so, we know what the md5 (contentID?) of the target rom file is
                                                 // If we don't recognize the md5, there's nothing we can do
                                             }

                                             if (m_library.getRomFileWithPathAndSize(fileInfo.filePath(),
                                                 fileInfo.size(), false)) {
                                                 spdlog::info("Skipping known file: {}",
                                                              fileInfo.filePath().toStdString());
                                                 continue;
                                             }

                                             auto romFile = RomFile(fileInfo.filePath());
                                             if (romFile.isValid()) {
                                                 m_library.addRomFile(romFile);
                                             }

                                             // metadata scan:
                                             // for each entry

                                             // look up firelight content id using content hash
                                             // that gets us release year, description... etc.

                                             // then look up at retroachievements using content hash
                                             // to do that we need a db table with the mapping
                                             // that gets us image
                                         }

                                         spdlog::info("Finished scanning directory: {}",
                                                      dirInfo.filePath().toStdString(),
                                                      dirInfo.lastModified().toString().toStdString());

                                         // update directory info

                                         if (subdirectories.isEmpty()) {
                                             m_pathQueueLock.lockForRead();
                                             queueEmpty = m_pathQueue.isEmpty();
                                             if (!queueEmpty) {
                                                 nextDirectory = m_pathQueue.dequeue();
                                                 m_scanQueuedByPath[nextDirectory] = false;
                                             }
                                             m_pathQueueLock.unlock();
                                         } else {
                                             nextDirectory = subdirectories.pop();
                                         }
                                     }

                                     for (const auto &knownFile: knownFiles) {
                                         if (!scannedFiles.contains(knownFile)) {
                                             spdlog::info("Removing file: {}", knownFile.toStdString());
                                             if (knownFile.contains("|")) {
                                                 auto parts = knownFile.split("|");
                                                 m_library.removeRomFile(parts[1], true, parts[0]);
                                             } else {
                                                 m_library.removeRomFile(knownFile, false, "");
                                             }
                                         }
                                     }

                                     m_scanRunning = false;
                                     emit scanFinished();
                                     spdlog::info("Scan complete");
                                     return true;
                                 }

        );

        // Check for files that ARE in the database but not in the filesystem
        // Calculate changeset between all directories - for example if removed from one but added to another
    }

    void LibraryScanner2::scanAll() {
        for (const auto &dir: m_library.getWatchedDirectories()) {
            queueScan(dir.path);
        }
        startScan();
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

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

        m_scanTimer.setInterval(1000);
        m_scanTimer.setSingleShot(true);
        m_scanTimer.callOnTimeout([&] {
            startScan();
        });

        connect(&m_watcher, &QFileSystemWatcher::directoryChanged,
                [&](const QString &path) {
                    queueScan(path);
                    m_scanTimer.start(); // TODO This might need to be restart
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
                                     emit scanningChanged();

                                     for (auto &romFile: m_library.getRomFiles()) {
                                         auto path = romFile.getFilePath();
                                         if (romFile.inArchive()) {
                                             path = romFile.getArchivePathName();
                                         }

                                         if (!QFileInfo::exists(path)) {
                                             spdlog::info("Removing missing file: {}",
                                                          path.toStdString());
                                             m_library.removeRomFile(romFile.getFilePath(), romFile.inArchive(),
                                                                     romFile.getArchivePathName());
                                         }
                                     }

                                     while (auto nextDirectory =
                                             getNextDirectory()) {
                                         scanDirectory(nextDirectory.value());

                                         spdlog::info("Finished scanning directory: {}",
                                                      nextDirectory.value().toStdString());
                                     }

                                     m_scanRunning = false;
                                     emit scanFinished();
                                     emit scanningChanged();
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

    std::optional<QString> LibraryScanner2::getNextDirectory() {
        m_pathQueueLock.lockForRead();
        if (!m_pathQueue.isEmpty()) {
            auto nextDirectory = m_pathQueue.dequeue();

            m_scanQueuedByPath[nextDirectory] = false;
            m_pathQueueLock.unlock();
            return {nextDirectory};
        }

        m_pathQueueLock.unlock();
        return std::nullopt;
    }

    void LibraryScanner2::scanDirectory(const QString &path) {
        QDirIterator iter(path,
                          QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

        auto dirInfo = QFileInfo(path);
        spdlog::info("Scanning directory: {} (last modified: {})",
                     dirInfo.filePath().toStdString(),
                     dirInfo.lastModified().toString().toStdString());

        // get directory info
        // if last modified is the same, skip


        while (iter.hasNext()) {
            if (m_shuttingDown) {
                return;
            }

            if (pathIsQueued(path)) {
                return;
            }

            const auto fileInfo = iter.nextFileInfo();
            if (fileInfo.isDir()) {
                queueScan(fileInfo.filePath());
                continue;
            }

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
    }

    bool LibraryScanner2::pathIsQueued(const QString &path) {
        m_pathQueueLock.lockForRead();
        const auto result = m_scanQueuedByPath[path];
        m_pathQueueLock.unlock();
        return result;
    }
} // namespace firelight::library

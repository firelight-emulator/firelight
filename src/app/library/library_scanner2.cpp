#include "library_scanner2.hpp"
#include <qdiriterator.h>
#include <QtConcurrent>
#include <archive.h>
#include <archive_entry.h>
#include <firelight/library/rom_file.hpp>
#include <rcheevos/rc_hash.h>

#include "../platform_metadata.hpp"


namespace firelight::library {
    LibraryScanner2::LibraryScanner2(IUserLibrary &library) : m_library(library) {
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
        auto m_consoleId = RC_CONSOLE_UNKNOWN;
        switch (platformId) {
            case 1:
                m_consoleId = RC_CONSOLE_GAMEBOY;
                break;
            case 2:
                m_consoleId = RC_CONSOLE_GAMEBOY_COLOR;
                break;
            case 3:
                m_consoleId = RC_CONSOLE_GAMEBOY_ADVANCE;
                break;
            case 4:
                m_consoleId = RC_CONSOLE_VIRTUAL_BOY;
                break;
            case 5:
                m_consoleId = RC_CONSOLE_NINTENDO;
                break;
            case 6:
                m_consoleId = RC_CONSOLE_SUPER_NINTENDO;
                break;
            case 7:
                m_consoleId = RC_CONSOLE_NINTENDO_64;
                break;
            case 10:
                m_consoleId = RC_CONSOLE_NINTENDO_DS;
                break;
            case 12:
                m_consoleId = RC_CONSOLE_MASTER_SYSTEM;
                break;
            case 13:
                m_consoleId = RC_CONSOLE_MEGA_DRIVE;
                break;
            case 14:
                m_consoleId = RC_CONSOLE_GAME_GEAR;
                break;
            case 15:
                m_consoleId = RC_CONSOLE_SATURN;
                break;
            case 16:
                m_consoleId = RC_CONSOLE_SEGA_32X;
                break;
            case 17:
                m_consoleId = RC_CONSOLE_SEGA_CD;
                break;
            case 18:
                m_consoleId = RC_CONSOLE_PLAYSTATION;
                break;
            case 19:
                m_consoleId = RC_CONSOLE_PLAYSTATION_2;
                break;
            case 20:
                m_consoleId = RC_CONSOLE_PSP;
                break;
            default:
                m_consoleId = RC_CONSOLE_UNKNOWN;
        }

        char hash[33];
        rc_hash_generate_from_buffer(hash, m_consoleId, reinterpret_cast<const uint8_t *>(fileData.data()),
                                     fileData.size());

        return QByteArray::fromHex(hash);

        // if (platformId == PlatformMetadata::PLATFORM_ID_NES) {
        //     if (fileData.startsWith("NES\x1A")) {
        //         fileData.remove(0, 16);
        //         return QCryptographicHash::hash(fileData, QCryptographicHash::Algorithm::Md5);
        //     }
        // }
        //
        // if (platformId == PlatformMetadata::PLATFORM_ID_SNES) {
        //     if (fileData.size() % 1024 == 512) {
        //         fileData.remove(0, 512);
        //         return QCryptographicHash::hash(fileData, QCryptographicHash::Algorithm::Md5);
        //     }
        // }
        //
        // if (platformId == PlatformMetadata::PLATFORM_ID_NINTENDO_DS) {
        //     int64_t offset = 0;
        //
        //     rc_hash_generate_from_buffer()
        //
        //     auto fileHeader = fileData.sliced(0, 512);
        //     const auto header = reinterpret_cast<uint8_t *>(fileHeader.data());
        //     uint32_t arm9_addr = header[0x20] | (header[0x21] << 8) | (header[0x22] << 16) | (header[0x23] << 24);
        //     uint32_t arm9_size = header[0x2C] | (header[0x2D] << 8) | (header[0x2E] << 16) | (header[0x2F] << 24);
        //     uint32_t arm7_addr = header[0x30] | (header[0x31] << 8) | (header[0x32] << 16) | (header[0x33] << 24);
        //     uint32_t arm7_size = header[0x3C] | (header[0x3D] << 8) | (header[0x3E] << 16) | (header[0x3F] << 24);
        //     uint32_t icon_addr = header[0x68] | (header[0x69] << 8) | (header[0x6A] << 16) | (header[0x6B] << 24);
        //
        //     if (arm9_size + arm7_size > 16 * 1024 * 1024) {
        //         /* TODO sanity check - code blocks are typically less than 1MB each - assume not a DS ROM */
        //         snprintf((char *) header, sizeof(header), "arm9 code size (%u) + arm7 code size (%u) exceeds 16MB",
        //                  arm9_size, arm7_size);
        //         return originalMd5;
        //     }
        //
        //     uint32_t hash_size = 0xA00;
        //     if (arm9_size > hash_size)
        //         hash_size = arm9_size;
        //     if (arm7_size > hash_size)
        //         hash_size = arm7_size;
        //
        //     QByteArray hashContent{};
        //
        //     hashContent.append(fileHeader.sliced(0, 0x160));
        //     hashContent.append(fileData.sliced(arm9_addr, arm9_size));
        //     hashContent.append(fileData.sliced(arm7_addr, arm7_size));
        //
        //     auto iconInfo = fileData.mid(icon_addr, 0xA00);
        //     if (iconInfo.length() < 0xA00) {
        //         iconInfo.append(QByteArray(0xA00 - iconInfo.length(), 0));
        //     }
        //
        //     hashContent.append(iconInfo);
        //
        //     printf("hash content size: %lld\n", hashContent.size());
        //     return QCryptographicHash::hash(hashContent, QCryptographicHash::Algorithm::Md5);
        // }
        //
        // if (platformId == PlatformMetadata::PLATFORM_ID_N64) {
        //     static const uint8_t Z64_SIGNATURE[4] = {0x80, 0x37, 0x12, 0x40};
        //     static const uint8_t V64_SIGNATURE[4] = {0x37, 0x80, 0x40, 0x12};
        //     static const uint8_t N64_SIGNATURE[4] = {0x40, 0x12, 0x37, 0x80};
        //
        //     if (memcmp(fileData.data(), V64_SIGNATURE, sizeof(V64_SIGNATURE)) == 0) {
        //         QByteArray other{};
        //         other.resize(fileData.length());
        //
        //         for (int i = 0; i < fileData.length(); i += 2) {
        //             other[i] = fileData[i + 1];
        //             other[i + 1] = fileData[i];
        //         }
        //         return QCryptographicHash::hash(other, QCryptographicHash::Algorithm::Md5);
        //     }
        //
        //     if (memcmp(fileData.data(), Z64_SIGNATURE, sizeof(Z64_SIGNATURE)) == 0) {
        //         return originalMd5;
        //     }
        //
        //     if (memcmp(fileData.data(), N64_SIGNATURE, sizeof(N64_SIGNATURE)) == 0) {
        //         QByteArray other{};
        //         other.resize(fileData.length());
        //         for (int i = 0; i < fileData.length(); i += 4) {
        //             other[i] = fileData[i + 3];
        //             other[i + 1] = fileData[i + 2];
        //             other[i + 2] = fileData[i + 1];
        //             other[i + 3] = fileData[i];
        //         }
        //         return QCryptographicHash::hash(other, QCryptographicHash::Algorithm::Md5);
        //     }
        // }

        // return originalMd5;
    }
} // namespace firelight::library

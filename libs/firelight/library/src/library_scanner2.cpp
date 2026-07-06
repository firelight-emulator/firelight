#include <firelight/library/library_scanner2.hpp>
#include <firelight/library/archive_reader.hpp>
#include <firelight/library/content_identifier.hpp>
#include <QDateTime>
#include <QDir>
#include <QtConcurrent>
#include <qcryptographichash.h>
#include <qdiriterator.h>
#include <spdlog/spdlog.h>

#include <firelight/platforms/platform_service.hpp>
#include <firelight/library/content_extensions.hpp>

#include <zlib.h>

namespace firelight::library {

void LibraryScanner2::persistDiscMembers(
    const int contentFileId,
    const std::vector<IdentifiedDiscMember> &members) {
  for (size_t i = 0; i < members.size(); ++i) {
    DiscMember member{.m_contentFileId = contentFileId,
                      .m_path = members[i].path,
                      .m_role = members[i].role,
                      .m_sortIndex = static_cast<int>(i)};
    m_library.create(member);
  }
}

LibraryScanner2::LibraryScanner2(IUserLibraryRepository &library,
                                 platforms::IPlatformService &platformService)
    : m_library(library), m_platformService(platformService) {
  m_threadPool.setMaxThreadCount(1);
  for (const auto &dir : m_library.getContentDirectories()) {
    watchPath(QString::fromStdString(dir.path));
  }

  m_scanTimer.setInterval(1000);
  m_scanTimer.setSingleShot(true);
  m_scanTimer.callOnTimeout([&] { startScan(); });

  // Safety net: a low-frequency full rescan catches anything the watcher missed
  // (network/removable drives, watch-limit overflow, dropped events). Cheap
  // thanks to the per-directory mtime short-circuit, and skipped during gameplay.
  m_periodicScanTimer.setInterval(PERIODIC_SCAN_INTERVAL_MS);
  m_periodicScanTimer.callOnTimeout([this] {
    if (!m_suspended) {
      scanAll();
    }
  });
  m_periodicScanTimer.start();

  connect(&m_watcher, &QFileSystemWatcher::directoryChanged,
          [&](const QString &path) {
            queueScan(path);
            m_scanTimer.start();
          });
}

LibraryScanner2::~LibraryScanner2() { m_shuttingDown = true; }

void LibraryScanner2::watchPath(const QString &path) {
  if (path.isEmpty() || m_watchedDirs.contains(path)) {
    return;
  }
  if (m_watchedDirs.size() >= MAX_WATCHED_DIRECTORIES) {
    if (!m_watchCapLogged) {
      spdlog::warn("Library watch limit ({}) reached; relying on periodic "
                   "rescans for the remaining folders",
                   MAX_WATCHED_DIRECTORIES);
      m_watchCapLogged = true;
    }
    return;
  }
  if (m_watcher.addPath(path)) {
    m_watchedDirs.insert(path);
  }
}

void LibraryScanner2::removePath(const QString &path) {
  m_watcher.removePath(path);
  m_watchedDirs.remove(path);
}

QStringList LibraryScanner2::watchedDirectories() const {
  return m_watcher.directories();
}

bool LibraryScanner2::isScanning() const { return m_scanRunning; }

void LibraryScanner2::scheduleWatch(const QString &path) {
  // QFileSystemWatcher must only be touched on the thread the scanner lives on
  // (the main thread); scanDirectory runs on a worker, so hop back via the
  // event loop.
  QMetaObject::invokeMethod(
      this, [this, path] { watchPath(path); }, Qt::QueuedConnection);
}

void LibraryScanner2::setScanningSuspended(const bool suspended) {
  m_suspended = suspended;
  if (!suspended) {
    // Resume: process anything that changed (or was queued) while suspended.
    QMetaObject::invokeMethod(
        this, [this] { scanAll(); }, Qt::QueuedConnection);
  }
}

QFuture<bool> LibraryScanner2::startScan() {
  if (m_scanRunning || m_suspended) {
    return QtConcurrent::run([] { return false; });
  }

  return QtConcurrent::run(&m_threadPool, [this] {
    // Scanning is best-effort background work; never contend with the emulator.
    QThread::currentThread()->setPriority(QThread::LowPriority);
    m_scanRunning = true;
    m_changesInScan = 0;
    emit scanningChanged();

    while (auto nextDirectory = getNextDirectory()) {
      scanDirectory(nextDirectory.value());

      spdlog::debug("Finished scanning directory: {}",
                    nextDirectory.value().toStdString());
    }

    auto allRoms = m_library.getContentFiles();
    for (auto &romFile : allRoms) {
      auto filePath = romFile.m_filePath;
      if (romFile.m_inArchive) {
        filePath = romFile.m_archivePathName;
      }

      if (!QFileInfo::exists(QString::fromStdString(filePath))) {
        spdlog::debug("Removing missing file: {}", filePath);
        m_library.deleteContentFile(romFile.m_id);
        ++m_changesInScan;
      }
    }

    m_scanRunning = false;
    // Only refresh the library UI when the scan actually changed something, so a
    // periodic no-op scan doesn't reset the list (losing scroll/selection).
    if (m_changesInScan > 0) {
      emit scanFinished();
    }
    emit scanningChanged();
    spdlog::info("Scan complete ({} change(s))", m_changesInScan.load());
    return true;
  });
}

void LibraryScanner2::scanAll() {
  for (const auto &dir : m_library.getContentDirectories()) {
    queueScan(QString::fromStdString(dir.path));
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
  const ContentIdentifier identifier(m_platformService);

  const QFileInfo dirInfo(path);
  const int64_t dirMtime = dirInfo.lastModified().toMSecsSinceEpoch();
  // If this directory is unchanged since we last scanned it, nothing was added,
  // removed, or renamed directly in it. We still re-descend into subdirectories
  // (their mtimes are checked independently), but skip the per-file DB lookups
  // and identification here -- that is what keeps periodic rescans cheap.
  const auto mtimeIt = m_dirMtimeByPath.find(path.toStdString());
  const bool unchanged =
      mtimeIt != m_dirMtimeByPath.end() && mtimeIt->second == dirMtime;

  // Whether this directory holds at least one game (so it's worth watching).
  bool dirHasContent = false;

  QDirIterator iter(path, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
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

    // An unchanged directory only needed a walk to discover its subdirectories.
    if (unchanged) {
      continue;
    }

    auto extension = fileInfo.suffix().toLower();
    const auto extensionStd = extension.toStdString();
    const bool isArchive = extension == "zip" || extension == "7z" ||
                           extension == "tar" || extension == "rar";
    const bool isDisc =
        firelight::library::isDiscExtension(extensionStd);

    // Disc images (and archives that may contain them) routinely exceed 1 GB,
    // so the size cap only applies to other files.
    if (fileInfo.size() > 1024LL * 1024 * 1024 && !isDisc && !isArchive) {
      spdlog::info("Skipping large file: {}",
                   fileInfo.filePath().toStdString());
      continue;
    }

    if (isArchive) {
      const std::string archivePath = fileInfo.filePath().toStdString();

      ArchiveReader(archivePath)
          .forEachEntry([&](const ArchiveReader::Entry &entry,
                            const std::function<std::vector<uint8_t>()>
                                &readBytes) {
            if (entry.size <= 0) {
              return;
            }
            const auto dotPos = entry.pathName.find_last_of('.');
            if (dotPos == std::string::npos) {
              return;
            }
            const std::string ext =
                QString::fromStdString(entry.pathName.substr(dotPos + 1))
                    .toLower()
                    .toStdString();

            if (firelight::library::isDiscExtension(ext)) {
              // Raw track files are pulled in via their cue/gdi sheet, so don't
              // classify them on their own.
              if (firelight::library::isDiscTrackExtension(ext)) {
                return;
              }
              if (m_library.getContentFileWithPathAndSize(entry.pathName,
                                                          entry.size, true)) {
                dirHasContent = true;
                return;
              }

              // Disc detection re-opens the archive to extract the disc set, so
              // we don't buffer the (potentially multi-GB) entry here.
              const auto identified = identifier.identifyInArchive(
                  entry.pathName, {}, static_cast<size_t>(entry.size),
                  archivePath);
              if (identified.valid) {
                auto romInfo = ContentFile{
                    .m_type = ContentType::Disc,
                    .m_fileSizeBytes = identified.fileSizeBytes,
                    .m_filePath = entry.pathName,
                    .m_fileMd5 = identified.fileMd5,
                    .m_fileCrc32 = ":)",
                    .m_inArchive = true,
                    .m_archivePathName = archivePath,
                    .m_platformId = identified.platformId,
                    .m_contentHash = identified.contentHash};
                m_library.create(romInfo);
                persistDiscMembers(romInfo.m_id, identified.discMembers);
                dirHasContent = true;
                ++m_changesInScan;
              }
              return;
            }

            if (m_platformService
                    .platformIdForExtension(ext) !=
                firelight::platforms::PlatformService::PLATFORM_ID_UNKNOWN) {
              if (m_library.getContentFileWithPathAndSize(entry.pathName,
                                                          entry.size, true)) {
                dirHasContent = true;
                return;
              }

              const std::vector<uint8_t> bytes = readBytes();
              const auto identified = identifier.identifyInArchive(
                  entry.pathName, bytes, bytes.size(), archivePath);
              if (identified.valid) {
                auto romInfo = ContentFile{
                    .m_type = ContentType::Cartridge,
                    .m_fileSizeBytes = identified.fileSizeBytes,
                    .m_filePath = entry.pathName,
                    .m_fileMd5 = identified.fileMd5,
                    .m_fileCrc32 = ":)",
                    .m_inArchive = true,
                    .m_archivePathName = archivePath,
                    .m_platformId = identified.platformId,
                    .m_contentHash = identified.contentHash};
                m_library.create(romInfo);
                dirHasContent = true;
                ++m_changesInScan;
              }
            }
          });
      continue;
    }

    if (extension == "ips" || extension == "bps" || extension == "ups" ||
        extension == "mod") {

      QFile file(fileInfo.filePath());
      if (!file.open(QIODevice::ReadOnly)) {
        spdlog::error("Failed to open patch file: {}",
                      fileInfo.filePath().toStdString());
        continue;
      }

      auto bytes = file.readAll();

      auto md5 =
          QCryptographicHash::hash(bytes, QCryptographicHash::Algorithm::Md5)
              .toHex();

      const auto crc = crc32(0L, nullptr, 0);
      auto fileCrc32 = QString::number(crc32(
          crc, reinterpret_cast<const Bytef *>(bytes.data()), bytes.size()));

      auto patch = PatchFile{};
      patch.m_filePath = fileInfo.filePath().toStdString();
      patch.m_fileSize = fileInfo.size();
      patch.m_fileMd5 = md5.toStdString();
      patch.m_fileCrc32 = fileCrc32.toStdString();
      patch.m_inArchive = false;

      m_library.create(patch);

      spdlog::debug("Skipping patch file for now: {}",
                    fileInfo.filePath().toStdString());

      file.close();
      continue;
    }

    if (m_platformService
                .platformIdForExtension(extensionStd) !=
            firelight::platforms::PlatformService::PLATFORM_ID_UNKNOWN ||
        firelight::library::isDiscExtension(extensionStd)) {
      // A raw disc track (bin/img/...) is classified via its sibling cue/gdi
      // sheet; skip the lone track when such a sheet exists in the directory.
      if (firelight::library::isDiscTrackExtension(extensionStd)) {
        const auto sheets = fileInfo.absoluteDir().entryList(
            {"*.cue", "*.gdi", "*.ccd", "*.m3u"}, QDir::Files);
        if (!sheets.isEmpty()) {
          continue;
        }
      }

      if (m_library.getContentFileWithPathAndSize(
              fileInfo.filePath().toStdString(), fileInfo.size(), false)) {
        spdlog::debug("Skipping known file: {}",
                      fileInfo.filePath().toStdString());
        dirHasContent = true;
        continue;
      }

      const auto identified =
          identifier.identify(fileInfo.filePath().toStdString());
      if (identified.valid) {
        auto romInfo = ContentFile{
            .m_type =
                identified.isDisc ? ContentType::Disc : ContentType::Cartridge,
            .m_fileSizeBytes = identified.fileSizeBytes,
            .m_filePath = fileInfo.filePath().toStdString(),
            .m_fileMd5 = identified.fileMd5,
            .m_fileCrc32 = ":)",
            .m_inArchive = false,
            .m_archivePathName = "",
            .m_platformId = identified.platformId,
            .m_contentHash = identified.contentHash};
        m_library.create(romInfo);
        persistDiscMembers(romInfo.m_id, identified.discMembers);
        dirHasContent = true;
        ++m_changesInScan;
      }
    }
  }

  // Remember this directory's mtime so an unchanged rescan can skip it, and watch
  // it if it holds games so future adds/removes here are caught instantly.
  m_dirMtimeByPath[path.toStdString()] = dirMtime;
  if (dirHasContent) {
    scheduleWatch(path);
  }
}

bool LibraryScanner2::pathIsQueued(const QString &path) {
  m_pathQueueLock.lockForRead();
  const auto result = m_scanQueuedByPath[path];
  m_pathQueueLock.unlock();
  return result;
}

} // namespace firelight::library

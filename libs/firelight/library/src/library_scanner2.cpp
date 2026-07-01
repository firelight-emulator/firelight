#include <firelight/library/library_scanner2.hpp>
#include <firelight/library/archive_reader.hpp>
#include <firelight/library/content_identifier.hpp>
#include <QDir>
#include <QtConcurrent>
#include <qcryptographichash.h>
#include <qdiriterator.h>
#include <spdlog/spdlog.h>

#include <platform_metadata.hpp>

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

LibraryScanner2::LibraryScanner2(IUserLibraryRepository &library) : m_library(library) {
  m_threadPool.setMaxThreadCount(1);
  for (const auto &dir : m_library.getWatchedDirectories()) {
    watchPath(dir.path);
  }

  m_scanTimer.setInterval(1000);
  m_scanTimer.setSingleShot(true);
  m_scanTimer.callOnTimeout([&] { startScan(); });

  connect(&m_watcher, &QFileSystemWatcher::directoryChanged,
          [&](const QString &path) {
            queueScan(path);
            m_scanTimer.start();
          });
}

LibraryScanner2::~LibraryScanner2() { m_shuttingDown = true; }

void LibraryScanner2::watchPath(const QString &path) {
  m_watcher.addPath(path);
}
void LibraryScanner2::removePath(const QString &path) {
  m_watcher.removePath(path);
}

QFuture<bool> LibraryScanner2::startScan() {
  if (m_scanRunning) {
    return QtConcurrent::run([] { return false; });
  }

  return QtConcurrent::run(&m_threadPool, [this] {
    QThread::currentThread()->setPriority(QThread::NormalPriority);
    m_scanRunning = true;
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
      }
    }

    m_scanRunning = false;
    emit scanFinished();
    emit scanningChanged();
    spdlog::info("Scan complete");
    return true;
  });
}

void LibraryScanner2::scanAll() {
  for (const auto &dir : m_library.getWatchedDirectories()) {
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
  const ContentIdentifier identifier;
  QDirIterator iter(path, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

  auto dirInfo = QFileInfo(path);
  spdlog::debug("Scanning directory: {} (last modified: {})",
                dirInfo.filePath().toStdString(),
                dirInfo.lastModified().toString().toStdString());

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

    auto extension = fileInfo.suffix().toLower();
    const auto extensionStd = extension.toStdString();
    const bool isArchive = extension == "zip" || extension == "7z" ||
                           extension == "tar" || extension == "rar";
    const bool isDisc =
        PlatformMetadata::isPossibleDiscExtension(extensionStd);

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

            if (PlatformMetadata::isPossibleDiscExtension(ext)) {
              // Raw track files are pulled in via their cue/gdi sheet, so don't
              // classify them on their own.
              if (PlatformMetadata::isDiscTrackExtension(ext)) {
                return;
              }
              if (m_library.getContentFileWithPathAndSize(
                      QString::fromStdString(entry.pathName), entry.size,
                      true)) {
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
              }
              return;
            }

            if (PlatformMetadata::isPossibleRomFileExtension(ext)) {
              if (m_library.getContentFileWithPathAndSize(
                      QString::fromStdString(entry.pathName), entry.size,
                      true)) {
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

    if (PlatformMetadata::isPossibleRomOrDiscExtension(extensionStd)) {
      // A raw disc track (bin/img/...) is classified via its sibling cue/gdi
      // sheet; skip the lone track when such a sheet exists in the directory.
      if (PlatformMetadata::isDiscTrackExtension(extensionStd)) {
        const auto sheets = fileInfo.absoluteDir().entryList(
            {"*.cue", "*.gdi", "*.ccd", "*.m3u"}, QDir::Files);
        if (!sheets.isEmpty()) {
          continue;
        }
      }

      if (m_library.getContentFileWithPathAndSize(fileInfo.filePath(),
                                              fileInfo.size(), false)) {
        spdlog::debug("Skipping known file: {}",
                      fileInfo.filePath().toStdString());
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
      }
    }
  }
}

bool LibraryScanner2::pathIsQueued(const QString &path) {
  m_pathQueueLock.lockForRead();
  const auto result = m_scanQueuedByPath[path];
  m_pathQueueLock.unlock();
  return result;
}

} // namespace firelight::library

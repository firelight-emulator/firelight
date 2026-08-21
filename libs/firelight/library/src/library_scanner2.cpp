#include <firelight/library/accepted_extensions.hpp>
#include <firelight/library/archive_reader.hpp>
#include <firelight/library/content_extensions.hpp>
#include <firelight/library/content_identifier.hpp>
#include <firelight/library/file_bytes.hpp>
#include <firelight/library/filename_tags.hpp>
#include <firelight/library/library_scanner2.hpp>
#include <firelight/platforms/platform_service.hpp>
#include <firelight/util/strings.hpp>

#include <QDateTime>
#include <QDir>
#include <QtConcurrent>
#include <qcryptographichash.h>
#include <qdiriterator.h>
#include <spdlog/spdlog.h>
#include <zlib.h>

namespace firelight::library {

namespace {
// The sheets whose contents say which raw tracks belong to a disc
const QStringList &sheetGlobs() {
  static const QStringList globs{"*.cue", "*.gdi", "*.ccd", "*.m3u"};
  return globs;
}

// Windows-authored cue sheets carry backslashes, and a sheet may name a track by a path
std::string baseNameOfToken(const std::string &token) {
  const auto separator = token.find_last_of("/\\");
  return strings::toLower(separator == std::string::npos ? token : token.substr(separator + 1));
}

// TODO
// Every track name the directory's sheets actually mention. Built once per directory: reading
// each sheet per track turns a folder of a few hundred games into tens of thousands of reads
std::set<std::string> referencedTrackNames(const QDir &directory) {
  std::set<std::string> names;

  for (const auto &sheetName : directory.entryList(sheetGlobs(), QDir::Files)) {
    const auto bytes = readAllBytes(directory.absoluteFilePath(sheetName).toStdString());

    for (const auto &candidate : DiscInspector::sheetFilenameCandidates(bytes)) {
      names.insert(baseNameOfToken(candidate));
    }
  }

  return names;
}

// The same question for an archive, whose sheets have to be read out of it first
std::set<std::string> referencedTrackNamesInArchive(const std::string &archivePath) {
  std::set<std::string> names;
  const ArchiveReader reader(archivePath);

  for (const auto &entry : reader.listEntries()) {
    const auto dot = entry.baseName.find_last_of('.');

    if (dot == std::string::npos || !isDiscSheetExtension(strings::toLower(entry.baseName.substr(dot + 1)))) {
      continue;
    }

    for (const auto &candidate : DiscInspector::sheetFilenameCandidates(reader.readEntryByPath(entry.pathName))) {
      names.insert(baseNameOfToken(candidate));
    }
  }

  return names;
}
} // namespace

void LibraryScanner2::persistDiscMembers(const int contentFileId, const std::vector<IdentifiedDiscMember> &members) {
  for (size_t i = 0; i < members.size(); ++i) {
    DiscMember member{.m_contentFileId = contentFileId,
                      .m_path = members[i].path,
                      .m_role = members[i].role,
                      .m_sortIndex = static_cast<int>(i)};
    m_library.create(member);
  }
}

void LibraryScanner2::recordDrop(const std::string &filePath, const std::string &archivePath,
                                 const std::string &extension, const size_t fileSizeBytes,
                                 const IdentifyOutcome outcome, const std::string &identifiedAs) {
  const ScanDrop drop{.filePath = filePath,
                      .archivePath = archivePath,
                      .extension = extension,
                      .fileSizeBytes = fileSizeBytes,
                      .outcome = outcome,
                      .identifiedAs = identifiedAs};

  // TODO
  // Deliberately not counted as a change: a drop does not alter the grid, and treating it as one
  // would make every periodic rescan of a folder holding one bad file reset the user's place
  if (m_library.recordScanDrop(drop)) {
    spdlog::warn("Could not catalogue {}{}", archivePath.empty() ? "" : archivePath + " -> ", filePath);
  }
}

LibraryScanner2::LibraryScanner2(IUserLibraryRepository &library, platforms::IPlatformService &platformService)
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
  // thanks to the per-directory mtime short-circuit, and skipped during gameplay
  m_periodicScanTimer.setInterval(PERIODIC_SCAN_INTERVAL_MS);
  m_periodicScanTimer.callOnTimeout([this] {
    if (!m_suspended) {
      scanAll();
    }
  });
  m_periodicScanTimer.start();

  connect(&m_watcher, &QFileSystemWatcher::directoryChanged, [&](const QString &path) {
    queueScan(path);
    m_scanTimer.start();
  });
}

LibraryScanner2::~LibraryScanner2() { m_shuttingDown = true; }

void LibraryScanner2::watchPath(const QString &path) {
  if (path.isEmpty()) {
    return;
  }

  // TODO
  // Qt drops the watch itself when a directory goes away, so a folder on a drive that was
  // unplugged is remembered here as watched while nothing is watching it. Left alone it is never
  // watched again for the life of the process, and still counts against the cap
  if (m_watchedDirs.contains(path) && !m_watcher.directories().contains(path)) {
    m_watchedDirs.remove(path);
  }

  if (m_watchedDirs.contains(path)) {
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

QStringList LibraryScanner2::watchedDirectories() const { return m_watcher.directories(); }

bool LibraryScanner2::isScanning() const { return m_scanRunning; }

void LibraryScanner2::scheduleWatch(const QString &path) {
  // QFileSystemWatcher must only be touched on the thread the scanner lives on
  // (the main thread); scanDirectory runs on a worker, so hop back via the
  // event loop
  QMetaObject::invokeMethod(this, [this, path] { watchPath(path); }, Qt::QueuedConnection);
}

void LibraryScanner2::setScanningSuspended(const bool suspended) {
  m_suspended = suspended;
  if (!suspended) {
    // Resume: process anything that changed (or was queued) while suspended
    QMetaObject::invokeMethod(this, [this] { scanAll(); }, Qt::QueuedConnection);
  }
}

LibraryScanner2::UnreachableRoots LibraryScanner2::unreachableRoots() const {
  UnreachableRoots roots;

  for (const auto &directory : m_library.getContentDirectories()) {
    if (!QFileInfo::exists(QString::fromStdString(directory.path))) {
      spdlog::info("Content directory is not available, leaving its files alone: {}", directory.path);
      roots.ids.insert(directory.id);
      roots.paths.push_back(directory.path);
    }
  }

  return roots;
}

bool LibraryScanner2::isUnderAnyRoot(const std::string &path, const std::vector<std::string> &roots) {
  const auto candidate = QDir::cleanPath(QString::fromStdString(path));

  return std::ranges::any_of(roots, [&candidate](const std::string &root) {
    const auto prefix = QDir::cleanPath(QString::fromStdString(root));

    // A separator on the end, so D:/Games does not claim D:/GamesArchive
    return candidate == prefix || candidate.startsWith(prefix + QLatin1Char('/'));
  });
}

QFuture<bool> LibraryScanner2::startScan() {
  if (m_scanRunning || m_suspended) {
    return QtConcurrent::run([] { return false; });
  }

  return QtConcurrent::run(&m_threadPool, [this] {
    // Scanning is best-effort background work; never contend with the emulator
    QThread::currentThread()->setPriority(QThread::LowPriority);
    m_scanRunning = true;
    m_changesInScan = 0;
    emit scanningChanged();

    while (auto nextDirectory = getNextDirectory()) {
      scanDirectory(nextDirectory.value());

      spdlog::debug("Finished scanning directory: {}", nextDirectory.value().toStdString());
    }

    const auto unreachable = unreachableRoots();

    auto allRoms = m_library.getContentFiles();
    for (auto &romFile : allRoms) {
      if (unreachable.ids.contains(romFile.m_contentDirectoryId)) {
        continue;
      }

      auto filePath = romFile.m_filePath;
      if (romFile.m_inArchive) {
        filePath = romFile.m_archivePathName;
      }

      const auto isOnDisk = QFileInfo::exists(QString::fromStdString(filePath));

      if (!isOnDisk && romFile.m_missingSince == 0) {
        spdlog::debug("Marking missing file: {}", filePath);
        m_library.markContentFileMissing(romFile.m_id);
        ++m_changesInScan;
      } else if (isOnDisk && romFile.m_missingSince != 0) {
        spdlog::debug("File is back: {}", filePath);
        m_library.reviveContentFile(romFile.m_id);
        ++m_changesInScan;
      }
    }

    // TODO
    // The record describes what is still wrong, so a file somebody deleted stops being reported
    // rather than being complained about forever
    for (const auto &drop : m_library.getScanDrops()) {
      const auto &onDisk = drop.archivePath.empty() ? drop.filePath : drop.archivePath;

      // A drop has no directory of its own, so its root is the one its path sits under
      if (isUnderAnyRoot(onDisk, unreachable.paths)) {
        continue;
      }

      if (!QFileInfo::exists(QString::fromStdString(onDisk))) {
        m_library.clearScanDrop(drop.filePath, drop.archivePath);
      }
    }

    m_scanRunning = false;
    // Only refresh the library UI when the scan actually changed something, so a
    // periodic no-op scan doesn't reset the list (losing scroll/selection)
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
  // and identification here -- that is what keeps periodic rescans cheap
  const auto mtimeIt = m_dirMtimeByPath.find(path.toStdString());
  const bool unchanged = mtimeIt != m_dirMtimeByPath.end() && mtimeIt->second == dirMtime;

  // Whether this directory holds at least one game (so it's worth watching)
  bool dirHasContent = false;

  // Only read when the directory turns out to hold a raw track
  std::optional<std::set<std::string>> looseTrackNames;

  const auto accepted = acceptedExtensions(m_platformService);

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

    // An unchanged directory only needed a walk to discover its subdirectories
    if (unchanged) {
      continue;
    }

    auto extension = fileInfo.suffix().toLower();
    const auto extensionStd = extension.toStdString();
    const bool isArchive = extension == "zip" || extension == "7z" || extension == "tar" || extension == "rar";
    const bool isDisc = firelight::library::isDiscExtension(extensionStd);

    // Disc images (and archives that may contain them) routinely exceed 1 GB,
    // so the size cap only applies to other files
    if (fileInfo.size() > 1024LL * 1024 * 1024 && !isDisc && !isArchive) {
      spdlog::info("Skipping large file: {}", fileInfo.filePath().toStdString());
      recordDrop(fileInfo.filePath().toStdString(), "", extensionStd, static_cast<size_t>(fileInfo.size()),
                 IdentifyOutcome::Unreadable);
      continue;
    }

    if (isArchive) {
      const std::string archivePath = fileInfo.filePath().toStdString();
      // Only read when the archive turns out to hold a raw track
      std::optional<std::set<std::string>> archiveTrackNames;

      ArchiveReader(archivePath)
          .forEachEntry([&](const ArchiveReader::Entry &entry, const std::function<std::vector<uint8_t>()> &readBytes) {
            if (entry.size <= 0) {
              return;
            }
            const auto dotPos = entry.pathName.find_last_of('.');
            if (dotPos == std::string::npos) {
              return;
            }
            const std::string ext = QString::fromStdString(entry.pathName.substr(dotPos + 1)).toLower().toStdString();

            if (firelight::library::isDiscExtension(ext)) {
              // A raw track is reached through the sheet naming it. One nothing names is a file
              // in its own right, and skipping it is how a zipped cartridge went missing
              if (firelight::library::isDiscTrackExtension(ext)) {
                if (!archiveTrackNames.has_value()) {
                  archiveTrackNames = referencedTrackNamesInArchive(archivePath);
                }

                if (archiveTrackNames->count(baseNameOfToken(entry.pathName)) > 0) {
                  return;
                }
              }
              if (m_library.getContentFileWithPathAndSize(entry.pathName, entry.size, true)) {
                dirHasContent = true;
                return;
              }

              // Disc detection re-opens the archive to extract the disc set, so
              // we don't buffer the (potentially multi-GB) entry here
              const auto identified =
                  identifier.identifyInArchive(entry.pathName, {}, static_cast<size_t>(entry.size), archivePath);
              if (identified.isIdentified()) {
                const auto tags = parseFilenameTags(entry.pathName);
                auto romInfo =
                    ContentFile{.m_type = ContentType::Disc,
                                .m_fileSizeBytes = identified.fileSizeBytes,
                                .m_filePath = entry.pathName,
                                .m_fileMd5 = identified.fileMd5,
                                // Not computed for content yet; DAT matching against content.db is what would need it
                                .m_fileCrc32 = "",
                                .m_inArchive = true,
                                .m_archivePathName = archivePath,
                                .m_platformId = identified.platformId,
                                .m_contentHash = identified.contentHash,
                                .m_discNumber = tags.discNumber,
                                .m_regions = tags.regions};
                m_library.create(romInfo);
                persistDiscMembers(romInfo.m_id, identified.discMembers);
                dirHasContent = true;
                ++m_changesInScan;
              } else {
                recordDrop(entry.pathName, archivePath, ext, static_cast<size_t>(entry.size), identified.outcome,
                           identified.identifiedAs);
              }
              return;
            }

            if (m_platformService.platformIdForExtension(ext) !=
                firelight::platforms::PlatformService::PLATFORM_ID_UNKNOWN) {
              if (m_library.getContentFileWithPathAndSize(entry.pathName, entry.size, true)) {
                dirHasContent = true;
                return;
              }

              const std::vector<uint8_t> bytes = readBytes();
              const auto identified = identifier.identifyInArchive(entry.pathName, bytes, bytes.size(), archivePath);
              if (identified.isIdentified()) {
                auto romInfo =
                    ContentFile{.m_type = ContentType::Cartridge,
                                .m_fileSizeBytes = identified.fileSizeBytes,
                                .m_filePath = entry.pathName,
                                .m_fileMd5 = identified.fileMd5,
                                // Not computed for content yet; DAT matching against content.db is what would need it
                                .m_fileCrc32 = "",
                                .m_inArchive = true,
                                .m_archivePathName = archivePath,
                                .m_platformId = identified.platformId,
                                .m_contentHash = identified.contentHash};
                m_library.create(romInfo);
                dirHasContent = true;
                ++m_changesInScan;
              } else {
                recordDrop(entry.pathName, archivePath, ext, bytes.size(), identified.outcome, identified.identifiedAs);
              }
            } else {
              m_library.countUnrecognizedExtension(ext);
            }
          });
      continue;
    }

    if (extension == "ips" || extension == "bps" || extension == "ups" || extension == "mod") {

      QFile file(fileInfo.filePath());
      if (!file.open(QIODevice::ReadOnly)) {
        spdlog::error("Failed to open patch file: {}", fileInfo.filePath().toStdString());
        continue;
      }

      auto bytes = file.readAll();

      auto md5 = QCryptographicHash::hash(bytes, QCryptographicHash::Algorithm::Md5).toHex();

      const auto crc = crc32(0L, nullptr, 0);
      auto fileCrc32 = QString::number(crc32(crc, reinterpret_cast<const Bytef *>(bytes.data()), bytes.size()));

      auto patch = PatchFile{};
      patch.m_filePath = fileInfo.filePath().toStdString();
      patch.m_fileSize = fileInfo.size();
      patch.m_fileMd5 = md5.toStdString();
      patch.m_fileCrc32 = fileCrc32.toStdString();
      patch.m_inArchive = false;

      m_library.create(patch);

      spdlog::debug("Skipping patch file for now: {}", fileInfo.filePath().toStdString());

      file.close();
      continue;
    }

    if (accepted.count(extensionStd) > 0) {
      // A raw disc track is reached through the sheet naming it. Asking whether any sheet
      // exists rather than whether one names this file hid every loose track in a folder that
      // happened to hold one unrelated cue
      if (firelight::library::isDiscTrackExtension(extensionStd)) {
        if (!looseTrackNames.has_value()) {
          looseTrackNames = referencedTrackNames(fileInfo.absoluteDir());
        }

        if (looseTrackNames->count(fileInfo.fileName().toLower().toStdString()) > 0) {
          continue;
        }
      }

      if (m_library.getContentFileWithPathAndSize(fileInfo.filePath().toStdString(), fileInfo.size(), false)) {
        spdlog::debug("Skipping known file: {}", fileInfo.filePath().toStdString());
        dirHasContent = true;
        continue;
      }

      // The table holds only what is still wrong, so a path that used to fail and now does not
      // stops being reported
      m_library.clearScanDrop(fileInfo.filePath().toStdString(), "");

      const auto identified = identifier.identify(fileInfo.filePath().toStdString());
      if (identified.isIdentified()) {
        const auto tags = parseFilenameTags(fileInfo.filePath().toStdString());
        auto romInfo =
            ContentFile{.m_type = identified.isDisc ? ContentType::Disc : ContentType::Cartridge,
                        .m_fileSizeBytes = identified.fileSizeBytes,
                        .m_filePath = fileInfo.filePath().toStdString(),
                        .m_fileMd5 = identified.fileMd5,
                        // Not computed for content yet; DAT matching against content.db is what would need it
                        .m_fileCrc32 = "",
                        .m_inArchive = false,
                        .m_archivePathName = "",
                        .m_platformId = identified.platformId,
                        .m_contentHash = identified.contentHash,
                        .m_discNumber = tags.discNumber,
                        .m_regions = tags.regions};
        m_library.create(romInfo);
        persistDiscMembers(romInfo.m_id, identified.discMembers);
        dirHasContent = true;
        ++m_changesInScan;
      } else {
        recordDrop(fileInfo.filePath().toStdString(), "", extensionStd, static_cast<size_t>(fileInfo.size()),
                   identified.outcome);
      }
    } else if (!extensionStd.empty()) {
      // TODO
      // Counted rather than listed: this is the answer to which formats people own, and a folder
      // of save files must not become a folder of rows
      m_library.countUnrecognizedExtension(extensionStd);
    }
  }

  // Remember this directory's mtime so an unchanged rescan can skip it, and watch
  // it if it holds games so future adds/removes here are caught instantly
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

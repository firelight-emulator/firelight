// TODO: NEEDS REVIEW
#include <firelight/library/accepted_extensions.hpp>
#include <firelight/library/archive_reader.hpp>
#include <firelight/library/content_discoverer.hpp>
#include <firelight/library/content_extensions.hpp>
#include <firelight/library/content_identifier.hpp>
#include <firelight/library/disc_set_service.hpp>
#include <firelight/library/file_bytes.hpp>
#include <firelight/library/filename_tags.hpp>
#include <firelight/library/library_scanner2.hpp>
#include <firelight/platforms/platform_service.hpp>
#include <firelight/util/strings.hpp>

#include <QDateTime>
#include <QDir>
#include <QtConcurrent>
#include <filesystem>
#include <qcryptographichash.h>
#include <qdiriterator.h>
#include <spdlog/spdlog.h>
#include <zlib.h>

namespace firelight::library {

void LibraryScanner2::persistContentFileTracks(const int contentFileId,
                                               const std::vector<IdentifiedDiscMember> &members) {
  for (size_t i = 0; i < members.size(); ++i) {
    ContentFileTrack track{
        .m_contentFileId = contentFileId, .m_path = members[i].path, .m_sortIndex = static_cast<int>(i)};
    m_library.create(track);
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

  if (m_library.recordScanDrop(drop)) {
    spdlog::warn("Could not catalog {}{}", archivePath.empty() ? "" : archivePath + " -> ", filePath);
  }
}

LibraryScanner2::LibraryScanner2(IUserLibraryRepository &library, platforms::IPlatformService &platformService,
                                 IPatchAssociator *patchAssociator, DiscSetService *discSets)
    : m_library(library), m_platformService(platformService),
      m_patchAssociator(patchAssociator == nullptr ? m_nullPatchAssociator : *patchAssociator), m_discSets(discSets) {
  m_threadPool.setMaxThreadCount(1);
  for (const auto &dir : m_library.getContentDirectories()) {
    watchPath(QString::fromStdString(dir.path));
  }

  m_scanTimer.setInterval(1000);
  m_scanTimer.setSingleShot(true);
  m_scanTimer.callOnTimeout([&] { startScan(); });

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

int LibraryScanner2::queuedDirectoryCount() const {
  QReadLocker locker(&m_pathQueueLock);
  return static_cast<int>(m_scanQueuedByPath.size());
}

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
  if (m_suspended) {
    return QtConcurrent::run([] { return false; });
  }

  // TODO
  // Claimed here rather than inside the worker: set there, a second caller passes the check
  // before the first has started and queues a pass with nothing to do
  bool idle = false;

  if (!m_scanRunning.compare_exchange_strong(idle, true)) {
    return QtConcurrent::run([] { return false; });
  }

  emit scanningChanged();

  return QtConcurrent::run(&m_threadPool, [this] {
    // Scanning is best-effort background work; never contend with the emulator
    QThread::currentThread()->setPriority(QThread::LowPriority);
    m_changesInCurrentScan = 0;

    while (auto nextDirectory = getNextDirectory()) {
      scanDirectory(nextDirectory.value());

      spdlog::debug("Finished scanning directory: {}", nextDirectory.value().toStdString());
    }

    const auto unreachable = unreachableRoots();

    auto allRoms = m_library.getRecordedFiles();
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
        ++m_changesInCurrentScan;
      } else if (isOnDisk && romFile.m_missingSince != 0) {
        spdlog::debug("File is back: {}", filePath);
        m_library.reviveContentFile(romFile.m_id);
        ++m_changesInCurrentScan;
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
    if (m_changesInCurrentScan > 0) {
      emit scanFinished();
    }
    emit scanningChanged();
    spdlog::info("Scan complete ({} change(s))", m_changesInCurrentScan.load());
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
  QWriteLocker locker(&m_pathQueueLock);

  const auto queued = m_scanQueuedByPath.find(path);

  if (queued != m_scanQueuedByPath.end() && queued->second) {
    return;
  }

  m_scanQueuedByPath[path] = true;
  m_pathQueue.enqueue(path);
}

std::optional<QString> LibraryScanner2::getNextDirectory() {
  // TODO
  // A write lock: taking one off the queue and marking it taken are both writes
  QWriteLocker locker(&m_pathQueueLock);

  if (m_pathQueue.isEmpty()) {
    return std::nullopt;
  }

  auto nextDirectory = m_pathQueue.dequeue();
  m_scanQueuedByPath[nextDirectory] = false;
  return {nextDirectory};
}

bool LibraryScanner2::catalog(const DiscoveredFile &file, const ContentIdentifier &identifier) {
  if (m_library.getContentFileWithPathAndSize(file.path, static_cast<int64_t>(file.sizeBytes), file.isInArchive())) {
    spdlog::debug("Skipping known file: {}", file.path);
    return true;
  }

  // The table holds only what is still wrong, so a path that used to fail and now does not stops
  // being reported
  m_library.clearScanDrop(file.path, file.archivePath);

  // TODO
  // A track is reached through the sheet naming it, so it is recorded as one and never hashed:
  // identifying it would cost the read and tell us nothing we would act on
  if (file.role == ContentRole::Track) {
    auto trackInfo = toContentFile(file, {});

    // TODO
    // The size the walk measured, because nothing identified this file and the next scan finds a
    // known file by its path and size together
    trackInfo.m_fileSizeBytes = file.sizeBytes;
    m_library.create(trackInfo);
    return true;
  }

  // TODO
  // Claimed before identifying, because a playlist whose first disc is absent has no hash at all
  // and what it says about the discs that are here is worth reading anyway
  if (file.role == ContentRole::Playlist) {
    auto playlistInfo = toContentFile(file, {});
    playlistInfo.m_fileSizeBytes = file.sizeBytes;
    m_library.create(playlistInfo);

    if (m_discSets == nullptr || file.isInArchive() || !claimPlaylist(file, identifier.membersNamedBy(file.path))) {
      spdlog::warn("Could not read the discs named by playlist {}", file.path);
    }

    ++m_changesInCurrentScan;
    return true;
  }

  // TODO
  // A disc is identified from its container rather than from a buffer, so its bytes are never read
  const auto identified =
      file.isInArchive()
          ? identifier.identifyInArchive(file.path, file.isDisc ? std::vector<uint8_t>{} : file.readBytes(),
                                         file.sizeBytes, file.archivePath)
          : identifier.identify(file.path);

  if (!identified.isIdentified()) {
    recordDrop(file.path, file.archivePath, file.extension, file.sizeBytes, identified.outcome,
               identified.identifiedAs);
    return false;
  }

  auto romInfo = toContentFile(file, identified);
  m_library.create(romInfo);
  persistContentFileTracks(romInfo.m_id, identified.discMembers);
  ++m_changesInCurrentScan;
  return true;
}

// TODO
// The discs a playlist names, in the order it names them. A sheet that names tracks rather than
// discs is a single disc's container and is catalogued like any other
bool LibraryScanner2::claimPlaylist(const DiscoveredFile &file, const std::vector<IdentifiedDiscMember> &members) {
  std::vector<std::string> discPaths;

  // TODO
  // Every line, whatever the file behind it turns out to be: a playlist names discs by definition,
  // and asking the extension instead drops a set written as .chd rather than .cue
  for (const auto &member : members) {
    // TODO
    // Sheet members are built with the platform's own separator while catalogued files carry
    // the one the walk produced, so the two only line up in the generic form
    discPaths.push_back(std::filesystem::path(member.path).generic_string());
  }

  if (discPaths.empty()) {
    return false;
  }

  return m_discSets->claimPlaylist(file.path, discPaths).has_value();
}

void LibraryScanner2::catalogPatch(const DiscoveredFile &file) {
  const auto bytes = file.readBytes();

  if (bytes.empty()) {
    spdlog::error("Failed to read patch file: {}", file.path);
    return;
  }

  const auto md5 = QCryptographicHash::hash(
                       QByteArray(reinterpret_cast<const char *>(bytes.data()), static_cast<qsizetype>(bytes.size())),
                       QCryptographicHash::Algorithm::Md5)
                       .toHex();
  const auto crc =
      crc32(crc32(0L, nullptr, 0), reinterpret_cast<const Bytef *>(bytes.data()), static_cast<uInt>(bytes.size()));

  auto patch = PatchFile{};
  patch.m_filePath = file.path;
  patch.m_fileSize = static_cast<int64_t>(file.sizeBytes);
  patch.m_fileMd5 = md5.toStdString();
  patch.m_fileCrc32 = QString::number(crc).toStdString();
  patch.m_inArchive = file.isInArchive();

  m_library.create(patch);
  m_patchAssociator.associate(patch);
}

void LibraryScanner2::scanDirectory(const QString &path) {
  const auto dirMtime = QFileInfo(path).lastModified().toMSecsSinceEpoch();

  const auto known = m_dirMtimeByPath.find(path.toStdString());
  const bool unchanged = known != m_dirMtimeByPath.end() && known->second == dirMtime;

  const ContentIdentifier identifier(m_platformService);
  const ContentDiscoverer discoverer(m_platformService);
  bool dirHasContent = false;

  const auto skipped = discoverer.walk(
      path.toStdString(), [&](const DiscoveredFile &file) { dirHasContent |= catalog(file, identifier); },
      [&] { return !m_shuttingDown && !pathIsQueued(path); }, !unchanged);

  for (const auto &subdirectory : skipped.subdirectories) {
    queueScan(QString::fromStdString(subdirectory));
  }

  // TODO
  // A walk that stopped part way saw only some of the directory, so recording its timestamp would
  // let the rest be skipped forever
  if (!skipped.completed) {
    return;
  }

  for (const auto &[extension, count] : skipped.unrecognizedExtensions) {
    for (auto i = 0; i < count; ++i) {
      m_library.countUnrecognizedExtension(extension);
    }
  }

  for (const auto &file : skipped.tooLarge) {
    spdlog::info("Skipping large file: {}", file.path);
    recordDrop(file.path, file.archivePath, file.extension, file.sizeBytes, IdentifyOutcome::Unreadable);
  }

  for (const auto &file : skipped.patches) {
    catalogPatch(file);
  }

  // Remember this directory's mtime so an unchanged rescan can skip it, and watch
  // it if it holds games so future adds/removes here are caught instantly
  m_dirMtimeByPath[path.toStdString()] = dirMtime;
  if (dirHasContent) {
    scheduleWatch(path);
  }
}

bool LibraryScanner2::pathIsQueued(const QString &path) const {
  // TODO
  // find rather than operator[], which would insert the path being asked about and make a
  // question a write
  QReadLocker locker(&m_pathQueueLock);

  const auto queued = m_scanQueuedByPath.find(path);
  return queued != m_scanQueuedByPath.end() && queued->second;
}

} // namespace firelight::library

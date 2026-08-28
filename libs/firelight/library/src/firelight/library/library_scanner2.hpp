// TODO: NEEDS REVIEW
#pragma once

#include <firelight/library/content_discoverer.hpp>
#include <firelight/library/content_identifier.hpp>
#include <firelight/library/disc_inspector.hpp>
#include <firelight/library/patch_associator.hpp>
#include <firelight/library/user_library_repository.hpp>

#include <QFileSystemWatcher>
#include <QFuture>
#include <QObject>
#include <QQueue>
#include <QReadWriteLock>
#include <QSet>
#include <QStringList>
#include <QTimer>
#include <atomic>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace firelight::platforms {
class IPlatformService;
}

namespace firelight::library {
class LibraryScanner2 : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool scanning MEMBER m_scanRunning NOTIFY scanningChanged)

public:
  LibraryScanner2(IUserLibraryRepository &library, platforms::IPlatformService &platformService,
                  IPatchAssociator *patchAssociator = nullptr);

  ~LibraryScanner2() override;

  void watchPath(const QString &path);

  void removePath(const QString &path);

  QFuture<bool> startScan();

  void scanAll();

  void setScanningSuspended(bool suspended);

  [[nodiscard]] QStringList watchedDirectories() const;
  [[nodiscard]] bool isScanning() const;

  [[nodiscard]] int queuedDirectoryCount() const;

signals:
  void scanFinished();

  void scanningChanged();

private:
  // Content roots that cannot be read right now like unplugged drives or deleted folders.
  // Ids for content files since they know their directory, paths for files that don't
  struct UnreachableRoots {
    std::unordered_set<int> ids;
    std::vector<std::string> paths;
  };

  [[nodiscard]] UnreachableRoots unreachableRoots() const;

  [[nodiscard]] static bool isUnderAnyRoot(const std::string &path, const std::vector<std::string> &roots);

  // Watching game folders (not the whole tree) keeps the count small; the
  // periodic rescan is the actual guarantee that changes are caught, so this is
  // just a rarely-hit backstop against exhausting OS watch handles
  static constexpr int MAX_WATCHED_DIRECTORIES = 256;
  static constexpr int PERIODIC_SCAN_INTERVAL_MS = 5 * 60 * 1000;

  QTimer m_scanTimer;
  // Low-frequency safety rescan for changes the watcher misses (network/
  // removable drives, watch-limit overflow, dropped events). Cheap because
  // unchanged directories are skipped by mtime
  QTimer m_periodicScanTimer;
  IUserLibraryRepository &m_library;
  platforms::IPlatformService &m_platformService;

  // TODO
  // What a discovered patch is offered to. Nothing matches patches to ROMs yet, so the default
  // takes them and does nothing rather than the scanner logging that it skipped what it just stored
  NullPatchAssociator m_nullPatchAssociator;
  IPatchAssociator &m_patchAssociator;
  bool m_shuttingDown = false;
  QFileSystemWatcher m_watcher;

  QSet<QString> m_watchedDirs;
  bool m_watchCapLogged = false;

  // Last-seen directory mtime (epoch ms) keyed by absolute path. Owned by the
  // single scan worker thread, so no lock is needed. Empty at launch, so the
  // first scan of the session is always a full/deep scan
  std::unordered_map<std::string, int64_t> m_dirMtimeByPath;
  std::map<QString, bool> m_scanQueuedByPath;
  std::atomic_bool m_scanRunning = false;

  std::atomic_bool m_suspended = false; // Scans are still queued while suspended
  // Content files added/removed during the in-flight scan. scanFinished (which
  // refreshes the library UI) is only emitted when this ends > 0, so a periodic
  // no-op scan never flickers the list
  std::atomic<int> m_changesInScan = 0;

  // Content files added/removed during the in-flight scan. scanFinished (which
  // refreshes the library UI) is only emitted when this ends > 0, so a periodic
  // no-op scan never flickers the list
  std::atomic<int> m_changesInCurrentScan = 0;
  QThreadPool m_threadPool;

  QQueue<QString> m_pathQueue;
  mutable QReadWriteLock m_pathQueueLock;

  void queueScan(const QString &path);

  std::optional<QString> getNextDirectory();

  void scanDirectory(const QString &path);

  // TODO
  // The four steps that turn one discovered file into a catalogue row: skip what is already
  // known, clear any stale drop, identify, and record either the row or why there is none.
  // Written once, so a field cannot be set for a loose file and forgotten for an archived one
  bool catalogue(const DiscoveredFile &file, const ContentIdentifier &identifier);

  // Hashes a patch and records it, wherever its bytes came from
  void catalogPatch(const DiscoveredFile &file);

  [[nodiscard]] bool pathIsQueued(const QString &path) const;

  // Registers `path` with the filesystem watcher. Safe to call from the scan
  // worker thread: it marshals onto the object's (main) thread, since
  // QFileSystemWatcher is not thread-safe
  void scheduleWatch(const QString &path);

  void persistDiscMembers(int contentFileId, const std::vector<IdentifiedDiscMember> &members);

  void recordDrop(const std::string &filePath, const std::string &archivePath, const std::string &extension,
                  size_t fileSizeBytes, IdentifyOutcome outcome, const std::string &identifiedAs = "");
};
} // namespace firelight::library

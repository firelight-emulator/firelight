#pragma once

#include <firelight/library/disc_inspector.hpp>
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
// Threading: a QObject on the GUI thread; QFileSystemWatcher/QTimer callbacks
// arrive there. A scan runs on a worker thread (startScan returns a QFuture), so
// scan state is guarded (QReadWriteLock/atomics); setScanningSuspended() is safe
// from any thread
class LibraryScanner2 : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool scanning MEMBER m_scanRunning NOTIFY scanningChanged)

public:
  LibraryScanner2(IUserLibraryRepository &library, platforms::IPlatformService &platformService);

  ~LibraryScanner2() override;

  void watchPath(const QString &path);

  void removePath(const QString &path);

  QFuture<bool> startScan();

  void scanAll();

  // Suspends scanning (e.g. while a game is running) and resumes it. Directory
  // changes seen while suspended are retained and processed on resume. Safe to
  // call from any thread
  void setScanningSuspended(bool suspended);

  // Diagnostics / tests: the directories currently registered with the watcher,
  // and whether a scan is in flight
  [[nodiscard]] QStringList watchedDirectories() const;
  [[nodiscard]] bool isScanning() const;

signals:
  void scanFinished();

  void scanningChanged();

private:
  // TODO
  // Content roots that cannot be read right now, which an unplugged drive and a deleted folder
  // both look like. Ids for content files, which carry the directory they came from; paths for
  // anything that only knows where it sits
  struct UnreachableRoots {
    std::unordered_set<int> ids;
    std::vector<std::string> paths;
  };

  // TODO
  // A root that cannot be read says nothing about the files under it: it answers exactly as a
  // deleted file does, so judging them would throw away the record of everything on that drive
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
  bool m_shuttingDown = false;
  QFileSystemWatcher m_watcher;
  // Directories currently registered with m_watcher (content roots + folders
  // that hold games). Main-thread-owned; bounded by MAX_WATCHED_DIRECTORIES
  QSet<QString> m_watchedDirs;
  bool m_watchCapLogged = false;
  // Last-seen directory mtime (epoch ms) keyed by absolute path. Owned by the
  // single scan worker thread, so no lock is needed. Empty at launch, so the
  // first scan of the session is always a full/deep scan
  std::unordered_map<std::string, int64_t> m_dirMtimeByPath;
  std::map<QString, bool> m_scanQueuedByPath;
  std::atomic_bool m_scanRunning = false;
  // Suspends scanning while a game is running. Queued directory changes are
  // retained and processed on resume
  std::atomic_bool m_suspended = false;
  // Content files added/removed during the in-flight scan. scanFinished (which
  // refreshes the library UI) is only emitted when this ends > 0, so a periodic
  // no-op scan never flickers the list
  std::atomic<int> m_changesInScan = 0;
  QThreadPool m_threadPool;

  QQueue<QString> m_pathQueue;
  QReadWriteLock m_pathQueueLock;

  void queueScan(const QString &path);

  std::optional<QString> getNextDirectory();

  void scanDirectory(const QString &path);

  bool pathIsQueued(const QString &path);

  // Registers `path` with the filesystem watcher. Safe to call from the scan
  // worker thread: it marshals onto the object's (main) thread, since
  // QFileSystemWatcher is not thread-safe
  void scheduleWatch(const QString &path);

  // Persists a disc set's member files against its primary ContentFile
  void persistDiscMembers(int contentFileId, const std::vector<IdentifiedDiscMember> &members);

  // TODO
  // Records a file that got this far and could not be catalogued, warning the first time each
  // one is seen so a rescan of the same folder does not repeat itself
  void recordDrop(const std::string &filePath, const std::string &archivePath, const std::string &extension,
                  size_t fileSizeBytes, IdentifyOutcome outcome, const std::string &identifiedAs = "");
};
} // namespace firelight::library

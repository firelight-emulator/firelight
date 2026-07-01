#pragma once

#include <firelight/library/disc_inspector.hpp>
#include <firelight/library/user_library_repository.hpp>
#include <QFileSystemWatcher>
#include <vector>
#include <QFuture>
#include <QObject>
#include <QQueue>
#include <QReadWriteLock>
#include <QTimer>

namespace firelight::library {
class LibraryScanner2 : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool scanning MEMBER m_scanRunning NOTIFY scanningChanged)

public:
  explicit LibraryScanner2(IUserLibraryRepository &library);

  ~LibraryScanner2() override;

  void watchPath(const QString &path);

  void removePath(const QString &path);

  QFuture<bool> startScan();

  void scanAll();

  QFuture<bool> scanPath(const QString &path);

signals:
  void scanStarted(const QString &path);

  void scanFinished();

  void scanningChanged();

private:
  QTimer m_scanTimer;
  IUserLibraryRepository &m_library;
  bool m_shuttingDown = false;
  QFileSystemWatcher m_watcher;
  std::map<QString, bool> m_scanQueuedByPath;
  std::atomic_bool m_scanRunning = false;
  QThreadPool m_threadPool;

  QQueue<QString> m_pathQueue;
  QReadWriteLock m_pathQueueLock;

  void queueScan(const QString &path);

  std::optional<QString> getNextDirectory();

  void scanDirectory(const QString &path);

  bool pathIsQueued(const QString &path);

  // Persists a disc set's member files against its primary ContentFile.
  void persistDiscMembers(int contentFileId,
                          const std::vector<IdentifiedDiscMember> &members);

  std::string calculateMd5(const QByteArray &data);
};
} // namespace firelight::library

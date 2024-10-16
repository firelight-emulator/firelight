#pragma once

#include <QFileSystemWatcher>
#include <QFuture>
#include <QObject>
#include <QQueue>
#include <QReadWriteLock>
#include <QTimer>
#include <firelight/library/user_library.hpp>

namespace firelight::library {
    class LibraryScanner2 : public QObject {
        Q_OBJECT
        Q_PROPERTY(bool scanning MEMBER m_scanRunning NOTIFY scanningChanged)

    public:
        explicit LibraryScanner2(IUserLibrary &library);

        ~LibraryScanner2() override;

        void watchPath(const QString &path);

        QFuture<bool> startScan();

        void scanAll();

        QFuture<bool> scanPath(const QString &path);

    signals:
        void scanStarted(const QString &path);

        void scanFinished();

        void scanningChanged();

    private:
        QTimer m_scanTimer;
        IUserLibrary &m_library;
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
    };
} // namespace firelight::library

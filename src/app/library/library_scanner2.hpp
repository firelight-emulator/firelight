#pragma once

#include <QFileSystemWatcher>
#include <QFuture>
#include <QObject>
#include <QQueue>
#include <QReadWriteLock>
#include <firelight/library/user_library.hpp>

namespace firelight::library {
    class LibraryScanner2 : public QObject {
        Q_OBJECT

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

    private:
        IUserLibrary &m_library;
        bool m_shuttingDown = false;
        QFileSystemWatcher m_watcher;
        std::map<QString, std::atomic_bool> m_scanQueuedByPath;
        std::atomic_bool m_scanRunning = false;
        QThreadPool m_threadPool;

        QQueue<QString> m_pathQueue;
        QReadWriteLock m_pathQueueLock;

        void queueScan(const QString &path);
    };
} // namespace firelight::library

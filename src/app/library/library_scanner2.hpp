#pragma once

#include <QFileSystemWatcher>
#include <QFuture>
#include <QObject>
#include <QQueue>
#include <QReadWriteLock>

namespace firelight::library {
    class LibraryScanner2 : public QObject {
        Q_OBJECT

    public:
        LibraryScanner2();

        ~LibraryScanner2();

        void watchPath(const QString &path);

        QFuture<bool> startScan();

        QFuture<bool> scanPath(const QString &path);

    signals:
        void scanStarted(const QString &path);

        void scanFinished(const QString &path);

    private:
        bool m_shuttingDown = false;
        QFileSystemWatcher m_watcher;
        std::map<QString, std::atomic_bool> m_scanQueuedByPath;
        std::atomic_bool m_scanRunning = false;
        QThreadPool m_threadPool;

        QQueue<QString> m_pathQueue;
        QReadWriteLock m_pathQueueLock;

        void queueScan(const QString &path);

        static QByteArray calculateContentId(int platformId, QByteArray &originalMd5, QByteArray &fileData);
    };
} // namespace firelight::library

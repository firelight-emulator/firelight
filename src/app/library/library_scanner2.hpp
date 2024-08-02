#pragma once

#include <qcryptographichash.h>
#include <QFileSystemWatcher>
#include <QFuture>
#include <QObject>

namespace firelight::library {
    class LibraryScanner2 : public QObject {
        Q_OBJECT

    public:
        LibraryScanner2();

        void watchPath(const QString &path);

        QFuture<bool> scanAllPaths();

        QFuture<bool> scanPath(const QString &path);

    signals:
        void scanStarted(const QString &path);

        void scanFinished(const QString &path);

    private:
        QFileSystemWatcher m_watcher;

        static QByteArray calculateContentId(int platformId, QByteArray &originalMd5, QByteArray &fileData);
    };
} // namespace firelight::library

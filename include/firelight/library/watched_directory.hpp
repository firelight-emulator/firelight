#pragma once

#include <QString>
#include <QDateTime>

namespace firelight::library {
    struct WatchedDirectory {
        int id = -1;
        QString path;
        int numFiles = 0;
        int numContentFiles = 0;
        QDateTime lastModified;
        bool recursive = true;
        unsigned createdAt = 0;
    };
} // namespace firelight::library

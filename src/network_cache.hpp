#pragma once

#include "http2config.hpp"

#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QQmlNetworkAccessManagerFactory>
#include <qstandardpaths.h>

class CachingNetworkAccessManagerFactory : public QQmlNetworkAccessManagerFactory
{
public:

    inline QNetworkAccessManager *create(QObject *parent) override
    {
        QNetworkAccessManager *networkAccessManager = new NetAccessManager(parent);
      networkAccessManager->setAutoDeleteReplies(true);
        auto diskCache = new QNetworkDiskCache(parent);
        QString directory = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
          + QLatin1StringView("/requestCache/");
        diskCache->setCacheDirectory(directory);
        networkAccessManager->setCache(diskCache);


        return networkAccessManager;
    }
};
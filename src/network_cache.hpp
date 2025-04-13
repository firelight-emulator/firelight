#pragma once

#include <QQmlNetworkAccessManagerFactory>


class CachingNetworkAccessManagerFactory : public QQmlNetworkAccessManagerFactory
{
public:

    inline QNetworkAccessManager *create(QObject *parent) override
    {
        QNetworkAccessManager *networkAccessManager = new QNetworkAccessManager(parent);
        QNetworkDiskCache *diskCache = new QNetworkDiskCache(parent);
        QString directory = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
          + QLatin1StringView("/requestCache/");
        diskCache->setCacheDirectory(directory);
        networkAccessManager->setCache(diskCache);

        return networkAccessManager;
    }
};
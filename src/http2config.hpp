#pragma once

#include <QNetworkAccessManager>
#include <QHttp2Configuration>


class NetAccessManager : public QNetworkAccessManager {
public:
  NetAccessManager(QObject *parent = nullptr) : QNetworkAccessManager(parent) {
    // m_config.setHuffmanCompressionEnabled(false);
    // m_config.setServerPushEnabled(true);
    // m_config.setMaxFrameSize(100000);

  }
protected:
  QNetworkReply *createRequest(Operation op,
                               const QNetworkRequest &originalReq,
                               QIODevice *outgoingData) override;
  //    inline QNetworkAccessManager *create(QObject *parent) override
  //    {
  //        QNetworkAccessManager *networkAccessManager = new
  //        QNetworkAccessManager(parent); auto diskCache = new
  //        QNetworkDiskCache(parent); QString directory =
  //        QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
  //          + QLatin1StringView("/requestCache/");
  //        diskCache->setCacheDirectory(directory);
  //        networkAccessManager->setCache(diskCache);
  //
  //
  //        return networkAccessManager;
  //    }

private:
  QHttp2Configuration m_config;
};
inline QNetworkReply *
NetAccessManager::createRequest(Operation op,
                                const QNetworkRequest &originalReq,
                                QIODevice *outgoingData) {
  auto request = originalReq;
  request.setHttp2Configuration(m_config);
  request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
  return QNetworkAccessManager::createRequest(op, request, outgoingData);


}
#pragma once
#include <cstdint>
#include <qbuffer.h>
#include <vector>
#include <QImage>
#include <QUrl>

struct SuspendPoint {
  std::vector<uint8_t> state;
  long long timestamp;
  QImage image;

  [[nodiscard]] QUrl getImageUrl() const {
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "png");
    QString base64 = QString::fromUtf8(byteArray.toBase64());
    return QString("data:image/png;base64,") + base64;
  }
};

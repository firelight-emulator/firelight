#pragma once

#include "firelight/image.hpp"

#include <QBuffer>
#include <QByteArray>
#include <QImage>

namespace firelight::gui {

// Converts a QImage to the Qt-free firelight::Image (PNG-encoded) at the GUI
// boundary, keeping domain types (SuspendPoint) free of Qt6::Gui
inline firelight::Image toImage(const QImage &image) {
  firelight::Image result;
  if (!image.isNull()) {
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    result.pngData.assign(bytes.begin(), bytes.end());
  }
  return result;
}

inline QImage toQImage(const firelight::Image &image) {
  QImage result;
  if (!image.isNull()) {
    result.loadFromData(image.pngData.data(), static_cast<int>(image.pngData.size()), "PNG");
  }
  return result;
}

} // namespace firelight::gui

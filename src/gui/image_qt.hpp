#pragma once

#include "firelight/image.hpp"
#include "firelight/video_frame.hpp"

#include <QBuffer>
#include <QByteArray>
#include <QImage>
#include <cstring>

namespace firelight::gui {

// Converts a QImage to the Qt-free firelight::Image (PNG-encoded)
inline Image toImage(const QImage &image) {
  Image result;
  if (!image.isNull()) {
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    result.pngData.assign(bytes.begin(), bytes.end());
  }
  return result;
}

inline QImage toQImage(const Image &image) {
  QImage result;
  if (!image.isNull()) {
    result.loadFromData(image.pngData.data(), static_cast<int>(image.pngData.size()), "PNG");
  }
  return result;
}

// Wraps a live frame's pixels as a QImage. The frame is tightly packed RGBA8888 the right way up, so
// this is a copy and nothing else
inline QImage toQImage(const VideoFrame &frame) {
  if (frame.isNull()) {
    return {};
  }

  return QImage(frame.pixels.data(), frame.width, frame.height, QImage::Format_RGBA8888_Premultiplied).copy();
}

// Normalises a QImage into a live frame which is tightly packed RGBA8888, whatever the source was
inline VideoFrame toVideoFrame(const QImage &image) {
  VideoFrame frame;

  if (image.isNull()) {
    return frame;
  }

  const auto normalized = image.format() == QImage::Format_RGBA8888_Premultiplied
                              ? image
                              : image.convertToFormat(QImage::Format_RGBA8888_Premultiplied);

  frame.width = normalized.width();
  frame.height = normalized.height();
  frame.pixels.resize(static_cast<size_t>(frame.width) * frame.height * 4);

  // Row by row, because a QImage's rows are padded to a four-byte boundary and a frame's are not
  const auto rowBytes = static_cast<size_t>(frame.width) * 4;
  for (auto y = 0; y < frame.height; ++y) {
    std::memcpy(frame.pixels.data() + static_cast<size_t>(y) * rowBytes, normalized.constScanLine(y), rowBytes);
  }

  return frame;
}

} // namespace firelight::gui

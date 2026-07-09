#include "image_utils.hpp"

#include <QColor>
#include <QImage>
#include <QImageReader>
#include <QUrl>
#include <algorithm>

namespace firelight::gui {

namespace {
QString resolvePath(const QString &url) {
  if (const QUrl parsed(url); parsed.scheme() == "qrc") {
    return ":" + parsed.path();
  } else if (parsed.isLocalFile()) {
    return parsed.toLocalFile();
  }
  return url;
}

// Average color over rows [y0, y1), skipping near-transparent pixels.
QColor averageBand(const QImage &img, const int y0, const int y1) {
  quint64 r = 0, g = 0, b = 0, count = 0;
  for (int y = y0; y < y1; ++y) {
    for (int x = 0; x < img.width(); ++x) {
      const QColor c = img.pixelColor(x, y);
      if (c.alpha() < 32) {
        continue;
      }
      r += c.red();
      g += c.green();
      b += c.blue();
      ++count;
    }
  }
  if (count == 0) {
    return {Qt::black};
  }
  return {static_cast<int>(r / count), static_cast<int>(g / count),
          static_cast<int>(b / count)};
}
} // namespace

QVariantMap ImageUtils::samplePalette(const QString &url) const {
  QVariantMap result;
  result["ok"] = false;

  QImageReader reader(resolvePath(url));
  reader.setAutoTransform(true);
  // A coarse average is all we need, so downscale large images (and gif first
  // frames) while reading to keep this cheap.
  if (const QSize orig = reader.size();
      orig.isValid() && (orig.width() > 128 || orig.height() > 128)) {
    reader.setScaledSize(orig.scaled(128, 128, Qt::KeepAspectRatio));
  }

  QImage img = reader.read();
  if (img.isNull()) {
    return result;
  }
  img = img.convertToFormat(QImage::Format_ARGB32);

  const int h = img.height();
  const int band = std::max(1, h / 3);

  result["top"] = averageBand(img, 0, band).name();
  result["bottom"] = averageBand(img, h - band, h).name();
  result["average"] = averageBand(img, 0, h).name();
  result["ok"] = true;
  return result;
}
} // namespace firelight::gui

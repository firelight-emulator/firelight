#include "media/media_service.hpp"

#include <QDateTime>
#include <QDir>
#include <spdlog/spdlog.h>
#include <utility>

namespace firelight::media {

MediaService::MediaService(QString mediaDirectory)
    : m_mediaDirectory(std::move(mediaDirectory)) {}

std::optional<QString> MediaService::saveScreenshot(const QString &contentHash,
                                                    const QImage &image) {
  if (image.isNull() || contentHash.isEmpty()) {
    return std::nullopt;
  }

  const QString dir = m_mediaDirectory + "/" + contentHash;
  if (!QDir().mkpath(dir)) {
    spdlog::warn("Could not create screenshot directory {}", dir.toStdString());
    return std::nullopt;
  }

  const QString path = dir + "/" +
                       QString::number(QDateTime::currentMSecsSinceEpoch()) +
                       ".png";
  if (!image.save(path, "PNG")) {
    spdlog::warn("Could not save screenshot to {}", path.toStdString());
    return std::nullopt;
  }

  spdlog::info("Saved screenshot to {}", path.toStdString());
  return path;
}

} // namespace firelight::media

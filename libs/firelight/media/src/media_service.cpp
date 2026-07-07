#include <firelight/media/media_service.hpp>

#include <firelight/media/clip_muxer.hpp>

#include <QDateTime>
#include <QDir>
#include <spdlog/spdlog.h>
#include <utility>

namespace firelight::media {

MediaService::MediaService(QString capturesDirectory)
    : m_capturesDirectory(std::move(capturesDirectory)) {}

std::optional<QString> MediaService::saveScreenshot(const QString &contentHash,
                                                    const QImage &image) {
  if (image.isNull() || contentHash.isEmpty()) {
    return std::nullopt;
  }

  const QString dir = m_capturesDirectory + "/screenshots/" + contentHash;
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

std::optional<QString> MediaService::saveClip(const QString &contentHash,
                                              const ClipSnapshot &snapshot) {
  if (snapshot.empty() || contentHash.isEmpty()) {
    return std::nullopt;
  }

  const QString dir = m_capturesDirectory + "/clips/" + contentHash;
  if (!QDir().mkpath(dir)) {
    spdlog::warn("Could not create clip directory {}", dir.toStdString());
    return std::nullopt;
  }

  const QString path = dir + "/" +
                       QString::number(QDateTime::currentMSecsSinceEpoch()) +
                       ".mp4";
  if (!ClipMuxer::writeMp4(snapshot, path)) {
    spdlog::warn("Could not save clip to {}", path.toStdString());
    return std::nullopt;
  }

  spdlog::info("Saved clip to {}", path.toStdString());
  return path;
}

} // namespace firelight::media

#include <firelight/media/media_service.hpp>

#include <firelight/media/clip_muxer.hpp>
#include <firelight/media/clip_thumbnailer.hpp>

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <spdlog/spdlog.h>
#include <utility>

namespace firelight::media {
namespace {
// The capture filename stem is its epoch-ms timestamp; fall back to the file's
// mtime if a file was named some other way.
int64_t timestampFromFile(const QString &filePath) {
  bool ok = false;
  const int64_t stamp = QFileInfo(filePath).completeBaseName().toLongLong(&ok);
  return ok ? stamp
            : QFileInfo(filePath).lastModified().toMSecsSinceEpoch();
}
} // namespace

MediaService::MediaService(QString capturesDirectory,
                           IGameCaptureRepository &captures)
    : m_capturesDirectory(std::move(capturesDirectory)), m_captures(captures) {}

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

  const int64_t timestamp = QDateTime::currentMSecsSinceEpoch();
  const QString path = dir + "/" + QString::number(timestamp) + ".png";
  if (!image.save(path, "PNG")) {
    spdlog::warn("Could not save screenshot to {}", path.toStdString());
    return std::nullopt;
  }

  spdlog::info("Saved screenshot to {}", path.toStdString());
  index(CaptureType::Screenshot, contentHash, path, path, timestamp);
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

  const int64_t timestamp = QDateTime::currentMSecsSinceEpoch();
  const QString path = dir + "/" + QString::number(timestamp) + ".mp4";
  if (!ClipMuxer::writeMp4(snapshot, path)) {
    spdlog::warn("Could not save clip to {}", path.toStdString());
    return std::nullopt;
  }

  spdlog::info("Saved clip to {}", path.toStdString());

  const QString poster = dir + "/" + QString::number(timestamp) + ".png";
  QString thumbnail = poster;
  if (!ClipThumbnailer::writePoster(path, poster)) {
    thumbnail.clear();
  }
  index(CaptureType::Clip, contentHash, path, thumbnail, timestamp);
  return path;
}

void MediaService::reconcile() {
  scanCaptureDir(m_capturesDirectory + "/screenshots", CaptureType::Screenshot,
                 "png");
  scanCaptureDir(m_capturesDirectory + "/clips", CaptureType::Clip, "mp4");

  // Prune index rows whose files were deleted outside the app.
  for (const auto &capture : m_captures.listAll()) {
    if (!QFileInfo::exists(QString::fromStdString(capture.filePath))) {
      m_captures.remove(capture.id);
    }
  }
}

void MediaService::index(CaptureType type, const QString &contentHash,
                         const QString &filePath, const QString &thumbnailPath,
                         int64_t timestamp) {
  GameCapture capture;
  capture.contentHash = contentHash.toStdString();
  capture.type = type;
  capture.filePath = filePath.toStdString();
  capture.thumbnailPath = thumbnailPath.toStdString();
  capture.timestamp = timestamp;
  m_captures.add(capture);
}

void MediaService::scanCaptureDir(const QString &base, CaptureType type,
                                  const QString &ext) {
  QDir root(base);
  if (!root.exists()) {
    return;
  }
  for (const auto &hash :
       root.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
    QDir gameDir(root.filePath(hash));
    for (const auto &name :
         gameDir.entryList(QStringList{"*." + ext}, QDir::Files)) {
      const QString filePath = gameDir.filePath(name);
      if (m_captures.existsForPath(filePath.toStdString())) {
        continue;
      }
      QString thumbnail = filePath;
      if (type == CaptureType::Clip) {
        thumbnail =
            gameDir.filePath(QFileInfo(name).completeBaseName() + ".png");
        if (!QFileInfo::exists(thumbnail) &&
            !ClipThumbnailer::writePoster(filePath, thumbnail)) {
          thumbnail.clear();
        }
      }
      index(type, hash, filePath, thumbnail, timestampFromFile(filePath));
    }
  }
}

} // namespace firelight::media

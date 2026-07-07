#pragma once

#include <firelight/media/clip_snapshot.hpp>
#include <firelight/media/game_capture_repository.hpp>

#include <QImage>
#include <QString>
#include <cstdint>
#include <optional>

namespace firelight::media {

// Owns writing gameplay captures to disk under a captures directory, split by
// type (screenshots/, clips/) and then by content hash so the gallery can group
// by game, and records each one in the capture index (captures.db).
class MediaService {
public:
  MediaService(QString capturesDirectory, IGameCaptureRepository &captures);

  // Saves `image` as a PNG under
  // <capturesDirectory>/screenshots/<contentHash>/<epochMs>.png, creating the
  // directory if needed, and indexes it. Returns the absolute path, or nullopt
  // if the image is null, the content hash is empty, or the write failed.
  std::optional<QString> saveScreenshot(const QString &contentHash,
                                        const QImage &image);

  // Muxes an Instant Replay `snapshot` to an mp4 under
  // <capturesDirectory>/clips/<contentHash>/<epochMs>.mp4, generates a poster
  // thumbnail, and indexes it. Returns the absolute path, or nullopt on empty
  // snapshot / missing content hash / write failure.
  std::optional<QString> saveClip(const QString &contentHash,
                                  const ClipSnapshot &snapshot);

  // Syncs the index with the captures directory: indexes files present on disk
  // but missing from the index (regenerating missing clip posters) and prunes
  // rows whose files were deleted. Safe to call at startup / on a manual refresh.
  void reconcile();

private:
  void index(CaptureType type, const QString &contentHash,
             const QString &filePath, const QString &thumbnailPath,
             int64_t timestamp);
  // Indexes not-yet-indexed *.ext files under <base>/<hash>/.
  void scanCaptureDir(const QString &base, CaptureType type,
                      const QString &ext);

  QString m_capturesDirectory;
  IGameCaptureRepository &m_captures;
};

} // namespace firelight::media

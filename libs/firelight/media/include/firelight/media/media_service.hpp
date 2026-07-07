#pragma once

#include <firelight/media/clip_snapshot.hpp>

#include <QImage>
#include <QString>
#include <optional>

namespace firelight::media {

// Owns writing gameplay captures to disk under a captures directory, split by
// type (screenshots/, clips/) and then by content hash so the gallery can group
// by game. The gallery DB index + change events are layered on in a later phase;
// this is the on-disk write.
class MediaService {
public:
  explicit MediaService(QString capturesDirectory);

  // Saves `image` as a PNG under
  // <capturesDirectory>/screenshots/<contentHash>/<epochMs>.png, creating the
  // directory if needed. Returns the absolute path, or nullopt if the image is
  // null, the content hash is empty, or the write failed.
  std::optional<QString> saveScreenshot(const QString &contentHash,
                                        const QImage &image);

  // Muxes an Instant Replay `snapshot` to an mp4 under
  // <capturesDirectory>/clips/<contentHash>/<epochMs>.mp4. Returns the absolute
  // path, or nullopt on empty snapshot / missing content hash / write failure.
  std::optional<QString> saveClip(const QString &contentHash,
                                  const ClipSnapshot &snapshot);

private:
  QString m_capturesDirectory;
};

} // namespace firelight::media

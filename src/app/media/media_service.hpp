#pragma once

#include <QImage>
#include <QString>
#include <optional>

namespace firelight::media {

// Owns writing captured media (currently screenshots) to disk, laid out per
// content hash so the gallery can group by game. The gallery DB index + change
// events are layered on in a later phase; this is the on-disk write.
class MediaService {
public:
  explicit MediaService(QString mediaDirectory);

  // Saves `image` as a PNG under <mediaDirectory>/<contentHash>/<epochMs>.png,
  // creating the directory if needed. Returns the absolute path, or nullopt if
  // the image is null, the content hash is empty, or the write failed.
  std::optional<QString> saveScreenshot(const QString &contentHash,
                                        const QImage &image);

private:
  QString m_mediaDirectory;
};

} // namespace firelight::media

#pragma once

#include <QString>

namespace firelight::media {

// Produces a gallery poster image for a clip by decoding its first frame
class ClipThumbnailer {
public:
  // Decodes the first frame of `mp4Path` and writes it to `pngPath` as PNG
  // Returns false if the file can't be opened/decoded/saved. Uses the ffmpeg
  // libraries the media module already links
  static bool writePoster(const QString &mp4Path, const QString &pngPath);
};

} // namespace firelight::media

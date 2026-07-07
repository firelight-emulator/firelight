#pragma once

#include <cstdint>
#include <string>

namespace firelight::media {

enum class CaptureType {
  Screenshot = 0,
  Clip = 1,
};

// One captured media file (a screenshot or a gameplay clip) indexed for the
// gallery. filePath is the .png/.mp4; thumbnailPath is the image shown in the
// grid — the screenshot itself, or a clip's poster frame.
struct GameCapture {
  int id = 0;
  std::string contentHash; // the game this capture belongs to
  CaptureType type = CaptureType::Screenshot;
  std::string filePath;
  std::string thumbnailPath;
  int64_t timestamp = 0; // capture time (epoch ms)
  bool favorite = false;
  int64_t createdAt = 0; // index insert time (epoch ms)
};

} // namespace firelight::media

#pragma once

#include <firelight/media/clip_snapshot.hpp>

#include <QString>

namespace firelight::media {

// TODO
// Writes a buffered clip to an mp4. The video is **stream-copied** (the H.264
// was already produced incrementally by ClipRecorder — no re-encode here) with
// PTS/DTS rebased so playback starts at zero. A pure function of the snapshot,
// so it can be unit-tested with synthetic input; all ffmpeg lives in the .cpp
//
// Audio muxing (PCM -> AAC) is a planned follow-up; `snap.pcm` is carried
// through so it can be added without touching the recorder or callers
class ClipMuxer {
public:
  [[nodiscard]] static bool writeMp4(const ClipSnapshot &snap,
                                     const QString &path);
};

} // namespace firelight::media

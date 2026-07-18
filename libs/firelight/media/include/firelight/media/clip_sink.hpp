#pragma once

#include <cstddef>
#include <cstdint>

class QImage;

namespace firelight::media {

// The narrow boundary the render + audio threads push into while Instant Replay
// is recording. Deliberately free of ffmpeg types so producers (the emulator
// renderer, AudioManager) and tests depend only on this abstraction — the
// concrete ClipRecorder that implements it owns all the encoding
class IClipSink {
public:
  virtual ~IClipSink() = default;

  // A finished video frame (RGBA8888) with a presentation time in milliseconds
  // since recording started. Cheap for the caller: the sink copies the pixels
  // into a small bounded queue and returns immediately
  virtual void pushVideoFrame(const QImage &frame, int64_t ptsMs) = 0;

  // Interleaved stereo int16 audio (numFrames frames == 2*numFrames samples)
  virtual void pushAudio(const int16_t *data, std::size_t numFrames) = 0;

  // Whether the sink currently wants frames. Producers may skip expensive work
  // (e.g. a GPU framebuffer readback) when this is false. Defaults to always
  // wanting frames; the netplay sender returns false until armed
  [[nodiscard]] virtual bool wantsFrames() const { return true; }
};

} // namespace firelight::media

#pragma once

#include <firelight/media/clip_sink.hpp>
#include <firelight/media/clip_snapshot.hpp>

#include <memory>

class QImage;

namespace firelight::media {

// Records the last N seconds of gameplay as a rolling window of *compressed*
// H.264 — never raw frames. Video frames pushed from the render thread are
// encoded on a private worker thread and appended to a bounded packet ring
// (trimmed whole-GOP so it always begins on a keyframe); audio is kept as a
// small rolling PCM ring. snapshot() lifts the current window out as a plain
// ClipSnapshot for the muxer. All ffmpeg + threading state is hidden in the
// impl, so this header pulls in no libav.
class ClipRecorder final : public IClipSink {
public:
  // `windowSeconds` is how much recent play to keep (the clip length).
  explicit ClipRecorder(int windowSeconds = 15);
  ~ClipRecorder() override;

  ClipRecorder(const ClipRecorder &) = delete;
  ClipRecorder &operator=(const ClipRecorder &) = delete;

  // Sets up the encoder + worker for a source of the given geometry/rate. May
  // be called again to restart for a new game. Returns false if no usable H.264
  // encoder is available. `fps`/`sampleRate` come from the core's AV info.
  bool start(int width, int height, int fps, int sampleRate, int channels = 2);

  // Flushes the encoder and joins the worker. Idempotent.
  void stop();

  // Drops the buffered window but keeps recording (e.g. on reset / disc swap).
  void reset();

  [[nodiscard]] bool isRecording() const;

  // Blocks until the worker has encoded every queued frame, so a following
  // snapshot() includes the most recent gameplay. Cheap (a few frames).
  void flush();

  // --- IClipSink (cheap producers; hand off + return) ---
  void pushVideoFrame(const QImage &frame, int64_t ptsMs) override;
  void pushAudio(const int16_t *data, std::size_t numFrames) override;

  // A consistent copy of the current window, video beginning on a keyframe.
  // Empty (`.empty()`) until at least one keyframe has been buffered.
  [[nodiscard]] ClipSnapshot snapshot() const;

private:
  struct Impl;
  std::unique_ptr<Impl> m_impl;
};

} // namespace firelight::media

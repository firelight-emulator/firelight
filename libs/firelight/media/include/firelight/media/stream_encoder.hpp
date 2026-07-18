#pragma once

#include "clip_sink.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

namespace firelight::media {

struct StreamEncoderConfig {
  int width = 0;
  int height = 0;
  int fps = 60;
  // Rate of the PCM handed to pushAudio (the core's rate); output is always
  // 48 kHz Opus
  int inputSampleRate = 48000;
  int channels = 2;
  int bitrateKbps = 4000;
};

// TODO
// Live game-stream encoder: H.264 (bitrate-capped, self-contained IDRs) +
// Opus, emitting packets continuously through callbacks — no ring, no file
// Video encodes on a worker thread (bounded hand-off queue, drop-oldest);
// audio encodes inline on the calling thread (Opus frames are tiny)
// Set callbacks before start(); they fire on internal threads
class StreamEncoder final : public IClipSink {
public:
  StreamEncoder();
  ~StreamEncoder() override;

  using VideoPacketCallback = std::function<void(
      std::vector<uint8_t> data, int64_t ptsMs, bool keyframe)>;
  using AudioPacketCallback =
      std::function<void(std::vector<uint8_t> data, int64_t ptsMs)>;

  void setVideoPacketCallback(VideoPacketCallback callback);
  void setAudioPacketCallback(AudioPacketCallback callback);

  bool start(const StreamEncoderConfig &config);
  void stop();
  [[nodiscard]] bool isRunning() const;

  // The next encoded frame becomes an IDR (join/reconnect recovery)
  void requestKeyframe();
  // H.264 SPS/PPS for the receiver's decoder setup
  [[nodiscard]] std::vector<uint8_t> extradata() const;
  // Blocks until queued video frames are encoded (tests)
  void flush();

  void pushVideoFrame(const QImage &frame, int64_t ptsMs) override;
  void pushAudio(const int16_t *data, std::size_t numFrames) override;

private:
  struct Impl;
  std::unique_ptr<Impl> m_impl;
};

} // namespace firelight::media

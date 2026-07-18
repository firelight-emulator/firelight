#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <span>
#include <vector>

namespace firelight::media {

// One decoded frame as raw pixels (RGBA8888, tightly packed). Deliberately
// Qt-free; the display layer wraps it however it likes
struct StreamVideoFrame {
  int width = 0;
  int height = 0;
  std::vector<uint8_t> rgba;
};

// TODO
// Decodes a live game stream (H.264 + Opus 48 kHz stereo). Synchronous:
// push* decodes on the calling thread and fires the callback before
// returning. Set callbacks before start
class StreamDecoder {
public:
  StreamDecoder();
  ~StreamDecoder();

  using VideoFrameCallback =
      std::function<void(StreamVideoFrame frame, int64_t ptsMs)>;
  using AudioCallback =
      std::function<void(std::vector<int16_t> interleaved, int64_t ptsMs)>;

  void setVideoFrameCallback(VideoFrameCallback callback);
  void setAudioCallback(AudioCallback callback);

  bool start(std::span<const uint8_t> videoExtradata);
  void stop();
  [[nodiscard]] bool isRunning() const;
  // Mid-stream config change (resolution switch): tear down and re-arm
  bool reset(std::span<const uint8_t> videoExtradata);

  void pushVideoPacket(std::span<const uint8_t> data, int64_t ptsMs);
  void pushAudioPacket(std::span<const uint8_t> data, int64_t ptsMs);

private:
  struct Impl;
  std::unique_ptr<Impl> m_impl;
};

} // namespace firelight::media

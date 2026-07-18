#pragma once

#include <firelight/media/clip_sink.hpp>
#include <firelight/media/stream_encoder.hpp>
#include <firelight/netplay/messages.hpp>
#include <firelight/netplay/session.hpp>

#include <atomic>
#include <functional>
#include <map>
#include <mutex>

namespace firelight::netplay {

// TODO
// The host side of the game stream: permanently installed as the emulator's
// netplay sink, it does nothing until armed. Armed, it encodes each frame +
// the audio tap (H.264/Opus) and fans the packets out to every connected
// peer, dropping video per-peer under backpressure and recovering with an
// IDR. Frames arrive on the render thread, audio on the audio path, packets
// leave on encoder threads
class HostStreamSender final : public media::IClipSink {
public:
  explicit HostStreamSender(NetplaySession &session);
  ~HostStreamSender() override;

  // Fired once per encoder start with the parameters receivers need
  void setConfigReadyCallback(std::function<void(StreamConfig)> callback);

  void arm();
  void disarm();
  [[nodiscard]] bool armed() const { return m_armed.load(); }

  // The core's audio rate, from the audio tee (varies per game)
  void setAudioSampleRate(int rate);

  void onPeerReady(PlayerId memberId, IPeerLink &link);
  void onPeerLost(PlayerId memberId);

  void pushVideoFrame(const QImage &frame, int64_t ptsMs) override;
  void pushAudio(const int16_t *data, std::size_t numFrames) override;
  [[nodiscard]] bool wantsFrames() const override { return m_armed.load(); }

private:
  void onVideoPacket(std::vector<uint8_t> data, int64_t ptsMs, bool keyframe);
  void onAudioPacket(std::vector<uint8_t> data, int64_t ptsMs);

  static constexpr size_t VIDEO_BACKPRESSURE_HIGH = 256 * 1024;
  static constexpr size_t VIDEO_BACKPRESSURE_LOW = 64 * 1024;

  NetplaySession &m_session;
  media::StreamEncoder m_encoder;
  std::function<void(StreamConfig)> m_configReady;

  std::atomic<bool> m_armed{false};
  std::atomic<int> m_audioSampleRate{48000};
  std::atomic<uint32_t> m_videoSeq{0};
  std::atomic<uint32_t> m_audioSeq{0};
  int m_frameWidth = 0;
  int m_frameHeight = 0;

  struct PeerState {
    IPeerLink *link = nullptr;
    bool droppingVideo = false;
  };
  std::mutex m_peersMutex;
  std::map<PlayerId, PeerState> m_peers;
};

} // namespace firelight::netplay

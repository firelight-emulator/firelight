#include "host_stream_sender.hpp"

#include <firelight/netplay/base64.hpp>
#include <firelight/netplay/stream_packets.hpp>

#include <QImage>
#include <spdlog/spdlog.h>

namespace firelight::netplay {

HostStreamSender::HostStreamSender(NetplaySession &session) : m_session(session) {
  m_encoder.setVideoPacketCallback([this](std::vector<uint8_t> data, const int64_t ptsMs, const bool keyframe) {
    onVideoPacket(std::move(data), ptsMs, keyframe);
  });
  m_encoder.setAudioPacketCallback(
      [this](std::vector<uint8_t> data, const int64_t ptsMs) { onAudioPacket(std::move(data), ptsMs); });
}

HostStreamSender::~HostStreamSender() { m_encoder.stop(); }

void HostStreamSender::setConfigReadyCallback(std::function<void(StreamConfig)> callback) {
  m_configReady = std::move(callback);
}

void HostStreamSender::arm() {
  m_videoSeq = 0;
  m_audioSeq = 0;
  m_frameWidth = 0;
  m_frameHeight = 0;
  m_armed = true;
}

void HostStreamSender::disarm() {
  m_armed = false;
  m_encoder.stop();
  std::lock_guard lock(m_peersMutex);
  for (auto &[id, peer] : m_peers) {
    peer.droppingVideo = false;
  }
}

void HostStreamSender::setAudioSampleRate(const int rate) {
  if (rate > 0) {
    m_audioSampleRate = rate;
  }
}

void HostStreamSender::onPeerReady(const PlayerId memberId, IPeerLink &link) {
  {
    std::lock_guard lock(m_peersMutex);
    m_peers[memberId] = PeerState{&link, false};
  }
  // A newcomer (or reconnector) can only decode from an IDR
  if (m_armed.load() && m_encoder.isRunning()) {
    m_encoder.requestKeyframe();
  }
}

void HostStreamSender::onPeerLost(const PlayerId memberId) {
  std::lock_guard lock(m_peersMutex);
  m_peers.erase(memberId);
}

void HostStreamSender::pushVideoFrame(const QImage &frame, const int64_t ptsMs) {
  if (!m_armed.load()) {
    return;
  }
  const int width = frame.width();
  const int height = frame.height();
  if (width <= 0 || height <= 0) {
    return;
  }

  if (!m_encoder.isRunning() || width != m_frameWidth || height != m_frameHeight) {
    if (!m_encoder.start({.width = width,
                          .height = height,
                          .fps = 60,
                          .inputSampleRate = m_audioSampleRate.load(),
                          .channels = 2,
                          .bitrateKbps = 4000})) {
      spdlog::warn("[Netplay] stream encoder failed to start");
      m_armed = false;
      return;
    }
    m_frameWidth = width;
    m_frameHeight = height;
    spdlog::info("[Netplay] streaming {}x{}", width, height);

    if (m_configReady) {
      m_configReady(StreamConfig{
          .codec = "h264",
          .extradataB64 = base64Encode(m_encoder.extradata()),
          .width = width,
          .height = height,
          .fps = 60,
          .audioCodec = "opus",
          .sampleRate = 48000,
          .channels = 2,
          .inputTickHz = 60,
      });
    }
  }
  m_encoder.pushVideoFrame(frame, ptsMs);
}

void HostStreamSender::pushAudio(const int16_t *data, const std::size_t numFrames) {
  if (!m_armed.load() || !m_encoder.isRunning()) {
    return;
  }
  m_encoder.pushAudio(data, numFrames);
}

void HostStreamSender::onVideoPacket(std::vector<uint8_t> data, const int64_t ptsMs, const bool keyframe) {
  const auto bytes = encodePacket(VideoPacket{
      .seq = m_videoSeq++,
      .ptsMs = static_cast<uint32_t>(ptsMs),
      .keyframe = keyframe,
      .configChanged = false,
      .payload = std::move(data),
  });

  std::lock_guard lock(m_peersMutex);
  for (auto &[id, peer] : m_peers) {
    if (!peer.link) {
      continue;
    }
    const auto buffered = peer.link->bufferedBytes(ChannelKind::Video);
    if (peer.droppingVideo) {
      if (buffered > VIDEO_BACKPRESSURE_LOW) {
        continue; // still draining
      }
      // Recovered: resume from the next IDR
      peer.droppingVideo = false;
      if (!keyframe) {
        m_encoder.requestKeyframe();
        continue;
      }
    } else if (buffered > VIDEO_BACKPRESSURE_HIGH && !keyframe) {
      peer.droppingVideo = true;
      continue;
    }
    peer.link->send(ChannelKind::Video, bytes);
  }
}

void HostStreamSender::onAudioPacket(std::vector<uint8_t> data, const int64_t ptsMs) {
  // Audio is never dropped: it's the receivers' clock
  const auto bytes = encodePacket(AudioPacket{
      .seq = m_audioSeq++,
      .ptsMs = static_cast<uint32_t>(ptsMs),
      .payload = std::move(data),
  });
  std::lock_guard lock(m_peersMutex);
  for (auto &[id, peer] : m_peers) {
    if (peer.link) {
      peer.link->send(ChannelKind::Audio, bytes);
    }
  }
}

} // namespace firelight::netplay

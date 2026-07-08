#include "guest_stream_receiver.hpp"

#include <firelight/input/input_frame.hpp>
#include <firelight/netplay/base64.hpp>
#include <firelight/netplay/stream_packets.hpp>

#include <spdlog/spdlog.h>

namespace firelight::netplay {

static_assert(RETROPAD_FRAME_BYTES == input::InputFrame::SERIALIZED_SIZE,
              "netplay wire format must match InputFrame's encoding");

GuestStreamReceiver::GuestStreamReceiver(
    NetplaySession &session,
    std::function<std::shared_ptr<IAudioOutput>()> audioFactory,
    libretro::IRetropadProvider *localPads)
    : m_session(session), m_audioFactory(std::move(audioFactory)),
      m_localPads(localPads) {
  m_decoder.setVideoFrameCallback(
      [this](media::StreamVideoFrame frame, const int64_t ptsMs) {
        m_lastVideoPtsMs = static_cast<uint32_t>(ptsMs);
        std::lock_guard lock(m_sinkMutex);
        if (m_frameSink) {
          m_frameSink(std::move(frame));
        }
      });
  m_decoder.setAudioCallback(
      [this](const std::vector<int16_t> &interleaved, int64_t) {
        std::lock_guard lock(m_audioMutex);
        if (!m_audioOutput && m_audioFactory) {
          m_audioOutput = m_audioFactory();
          if (m_audioOutput) {
            m_audioOutput->initialize(48000);
          }
        }
        if (m_audioOutput) {
          m_audioOutput->receive(interleaved.data(), interleaved.size() / 2);
        }
      });
}

GuestStreamReceiver::~GuestStreamReceiver() { stopPlaying(); }

void GuestStreamReceiver::setFrameSink(
    std::function<void(media::StreamVideoFrame)> sink) {
  std::lock_guard lock(m_sinkMutex);
  m_frameSink = std::move(sink);
}

void GuestStreamReceiver::configure(const StreamConfig &config) {
  const auto extradata = base64Decode(config.extradataB64);
  if (!extradata) {
    spdlog::warn("[Netplay] stream config carried invalid extradata");
    return;
  }
  if (config.inputTickHz > 0) {
    m_inputTickHz = config.inputTickHz;
  }
  if (!m_decoder.reset(*extradata)) {
    spdlog::warn("[Netplay] stream decoder failed to start");
    return;
  }
  spdlog::info("[Netplay] receiving {}x{} stream", config.width, config.height);
}

void GuestStreamReceiver::startPlaying(const int platformId) {
  if (m_playing.exchange(true)) {
    return;
  }
  m_platformId = platformId;
  m_inputSeq = 0;
  m_inputThread = std::thread([this] { inputTickLoop(); });
}

void GuestStreamReceiver::stopPlaying() {
  if (m_playing.exchange(false)) {
    if (m_inputThread.joinable()) {
      m_inputThread.join();
    }
  }
  m_decoder.stop();
  std::lock_guard lock(m_audioMutex);
  m_audioOutput.reset();
}

void GuestStreamReceiver::onVideoPacket(std::span<const uint8_t> data) {
  const auto packet = decodeVideoPacket(data);
  if (packet && m_decoder.isRunning()) {
    m_decoder.pushVideoPacket(packet->payload, packet->ptsMs);
  }
}

void GuestStreamReceiver::onAudioPacket(std::span<const uint8_t> data) {
  const auto packet = decodeAudioPacket(data);
  if (packet && m_decoder.isRunning()) {
    m_decoder.pushAudioPacket(packet->payload, packet->ptsMs);
  }
}

void GuestStreamReceiver::inputTickLoop() {
  std::deque<RetropadFrameBytes> recent;
  while (m_playing.load()) {
    const auto tickHz = std::max(m_inputTickHz.load(), 10);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000 / tickHz));
    if (!m_playing.load() || !m_localPads) {
      continue;
    }
    const auto pad = m_localPads->getRetropadForPlayerIndex(0);
    if (!pad) {
      continue;
    }
    const auto frame =
        input::captureJoypadFrame(*pad, m_platformId.load(), 1);

    recent.push_front(frame.serialize());
    while (recent.size() > 3) {
      recent.pop_back();
    }

    InputPacket packet;
    packet.seq = m_inputSeq++;
    packet.displayedPtsMs = m_lastVideoPtsMs.load();
    packet.localPadIndex = 0;
    packet.frames.assign(recent.begin(), recent.end());
    m_session.sendToHost(ChannelKind::Input, encodePacket(packet));
  }
}

} // namespace firelight::netplay

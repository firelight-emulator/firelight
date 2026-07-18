#include <firelight/netplay/stream_packets.hpp>

#include <cstring>

namespace firelight::netplay {

namespace {
constexpr uint8_t FLAG_KEYFRAME = 1u << 0;
constexpr uint8_t FLAG_CONFIG_CHANGED = 1u << 1;

void put32(std::vector<uint8_t> &out, const uint32_t value) {
  out.push_back(static_cast<uint8_t>(value & 0xFF));
  out.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
  out.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
  out.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
}

uint32_t get32(std::span<const uint8_t> data, const size_t offset) {
  return static_cast<uint32_t>(data[offset]) | (static_cast<uint32_t>(data[offset + 1]) << 8) |
         (static_cast<uint32_t>(data[offset + 2]) << 16) | (static_cast<uint32_t>(data[offset + 3]) << 24);
}
} // namespace

std::vector<uint8_t> encodePacket(const VideoPacket &packet) {
  std::vector<uint8_t> out;
  out.reserve(13 + packet.payload.size());
  put32(out, packet.seq);
  put32(out, packet.ptsMs);
  uint8_t flags = 0;
  if (packet.keyframe) {
    flags |= FLAG_KEYFRAME;
  }
  if (packet.configChanged) {
    flags |= FLAG_CONFIG_CHANGED;
  }
  out.push_back(flags);
  put32(out, static_cast<uint32_t>(packet.payload.size()));
  out.insert(out.end(), packet.payload.begin(), packet.payload.end());
  return out;
}

std::optional<VideoPacket> decodeVideoPacket(std::span<const uint8_t> data) {
  if (data.size() < 13) {
    return std::nullopt;
  }
  VideoPacket packet;
  packet.seq = get32(data, 0);
  packet.ptsMs = get32(data, 4);
  const uint8_t flags = data[8];
  packet.keyframe = (flags & FLAG_KEYFRAME) != 0;
  packet.configChanged = (flags & FLAG_CONFIG_CHANGED) != 0;
  const auto payloadLen = get32(data, 9);
  if (data.size() < 13 + payloadLen) {
    return std::nullopt;
  }
  packet.payload.assign(data.begin() + 13, data.begin() + 13 + payloadLen);
  return packet;
}

std::vector<uint8_t> encodePacket(const AudioPacket &packet) {
  std::vector<uint8_t> out;
  out.reserve(8 + packet.payload.size());
  put32(out, packet.seq);
  put32(out, packet.ptsMs);
  out.insert(out.end(), packet.payload.begin(), packet.payload.end());
  return out;
}

std::optional<AudioPacket> decodeAudioPacket(std::span<const uint8_t> data) {
  if (data.size() < 8) {
    return std::nullopt;
  }
  AudioPacket packet;
  packet.seq = get32(data, 0);
  packet.ptsMs = get32(data, 4);
  packet.payload.assign(data.begin() + 8, data.end());
  return packet;
}

std::vector<uint8_t> encodePacket(const InputPacket &packet) {
  std::vector<uint8_t> out;
  out.reserve(10 + packet.frames.size() * RETROPAD_FRAME_BYTES);
  put32(out, packet.seq);
  put32(out, packet.displayedPtsMs);
  out.push_back(packet.localPadIndex);
  out.push_back(static_cast<uint8_t>(packet.frames.size()));
  for (const auto &frame : packet.frames) {
    out.insert(out.end(), frame.begin(), frame.end());
  }
  return out;
}

std::optional<InputPacket> decodeInputPacket(std::span<const uint8_t> data) {
  if (data.size() < 10) {
    return std::nullopt;
  }
  InputPacket packet;
  packet.seq = get32(data, 0);
  packet.displayedPtsMs = get32(data, 4);
  packet.localPadIndex = data[8];
  const uint8_t count = data[9];
  if (data.size() < 10 + static_cast<size_t>(count) * RETROPAD_FRAME_BYTES) {
    return std::nullopt;
  }
  packet.frames.resize(count);
  for (uint8_t i = 0; i < count; ++i) {
    std::memcpy(packet.frames[i].data(), data.data() + 10 + static_cast<size_t>(i) * RETROPAD_FRAME_BYTES,
                RETROPAD_FRAME_BYTES);
  }
  return packet;
}

} // namespace firelight::netplay

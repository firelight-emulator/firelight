#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace firelight::netplay {

// TODO
// Size of one serialized retropad frame (input::InputFrame's wire format)
// Carried as opaque bytes so this lib doesn't depend on the input module; the
// app glue static_asserts the two constants agree
inline constexpr size_t RETROPAD_FRAME_BYTES = 10;
using RetropadFrameBytes = std::array<uint8_t, RETROPAD_FRAME_BYTES>;

// Binary packets for the Video/Audio/Input channels (little-endian headers)

struct VideoPacket {
  uint32_t seq = 0;
  uint32_t ptsMs = 0;
  bool keyframe = false;
  bool configChanged = false;
  std::vector<uint8_t> payload;
};

struct AudioPacket {
  uint32_t seq = 0;
  uint32_t ptsMs = 0;
  std::vector<uint8_t> payload;
};

// TODO
// Guest -> host controller state. frames are newest-first with a few older
// duplicates for loss tolerance; the host applies the highest unseen seq
// displayedPtsMs is the video pts the guest was watching at capture time (the
// frame reference host-side rollback would use; unused in v1)
struct InputPacket {
  uint32_t seq = 0;
  uint32_t displayedPtsMs = 0;
  uint8_t localPadIndex = 0;
  std::vector<RetropadFrameBytes> frames;
};

[[nodiscard]] std::vector<uint8_t> encodePacket(const VideoPacket &packet);
[[nodiscard]] std::vector<uint8_t> encodePacket(const AudioPacket &packet);
[[nodiscard]] std::vector<uint8_t> encodePacket(const InputPacket &packet);

[[nodiscard]] std::optional<VideoPacket>
decodeVideoPacket(std::span<const uint8_t> data);
[[nodiscard]] std::optional<AudioPacket>
decodeAudioPacket(std::span<const uint8_t> data);
[[nodiscard]] std::optional<InputPacket>
decodeInputPacket(std::span<const uint8_t> data);

} // namespace firelight::netplay

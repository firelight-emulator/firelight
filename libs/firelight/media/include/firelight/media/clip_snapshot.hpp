#pragma once

#include <cstdint>
#include <vector>

namespace firelight::media {

// One already-encoded video packet lifted out of the rolling buffer. Times are
// in microseconds so the muxer can rebase them without knowing the codec
struct EncodedPacket {
  std::vector<uint8_t> data;
  int64_t ptsUs = 0;
  int64_t dtsUs = 0;
  bool keyframe = false;
};

// TODO
// A self-contained, ffmpeg-free description of a clip ready to be written to an
// mp4. `video` is a run of H.264 packets that begins on a keyframe; `extradata`
// is the codec's SPS/PPS needed to set up the output stream when stream-copying
// `pcm` is interleaved stereo int16 covering the same window. Because it carries
// no ffmpeg state, ClipMuxer::writeMp4 is a pure function of this struct and can
// be unit-tested with synthetic input
struct ClipSnapshot {
  std::vector<EncodedPacket> video;
  std::vector<uint8_t> extradata; // H.264 SPS/PPS (global header)
  std::vector<int16_t> pcm;
  int fps = 60;
  int sampleRate = 48000;
  int channels = 2;
  int width = 0;
  int height = 0;

  [[nodiscard]] bool empty() const { return video.empty(); }
};

} // namespace firelight::media

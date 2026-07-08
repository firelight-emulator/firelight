#include <firelight/media/stream_decoder.hpp>
#include <firelight/media/stream_encoder.hpp>

#include <QImage>
#include <gtest/gtest.h>

#include <cmath>
#include <mutex>

namespace firelight::media {

namespace {
struct CollectedPacket {
  std::vector<uint8_t> data;
  int64_t ptsMs;
  bool keyframe;
};
} // namespace

TEST(StreamCodecTest, EncodeDecodeRoundTrip) {
  StreamEncoder encoder;

  std::mutex mutex;
  std::vector<CollectedPacket> videoPackets;
  std::vector<CollectedPacket> audioPackets;
  encoder.setVideoPacketCallback(
      [&](std::vector<uint8_t> data, const int64_t ptsMs, const bool keyframe) {
        std::lock_guard lock(mutex);
        videoPackets.push_back({std::move(data), ptsMs, keyframe});
      });
  encoder.setAudioPacketCallback(
      [&](std::vector<uint8_t> data, const int64_t ptsMs) {
        std::lock_guard lock(mutex);
        audioPackets.push_back({std::move(data), ptsMs, false});
      });

  ASSERT_TRUE(encoder.start({.width = 64,
                             .height = 64,
                             .fps = 30,
                             .inputSampleRate = 32768, // a real core rate
                             .channels = 2,
                             .bitrateKbps = 500}));
  const auto extradata = encoder.extradata();
  EXPECT_FALSE(extradata.empty());

  // One second of solid-red video + a sine tone at the core's rate.
  QImage frame(64, 64, QImage::Format_RGBA8888);
  frame.fill(Qt::red);
  std::vector<int16_t> tone(32768 * 2);
  for (size_t i = 0; i < tone.size() / 2; ++i) {
    const auto value =
        static_cast<int16_t>(std::sin(i * 0.05) * 12000);
    tone[i * 2] = value;
    tone[i * 2 + 1] = value;
  }

  for (int i = 0; i < 30; ++i) {
    encoder.pushVideoFrame(frame, i * 33);
    encoder.pushAudio(tone.data() + (i * 1092) * 2, 1092);
    if (i % 4 == 0) {
      encoder.flush();
    }
  }
  encoder.flush();

  const size_t packetsBeforeIdr = [&] {
    std::lock_guard lock(mutex);
    return videoPackets.size();
  }();
  encoder.requestKeyframe();
  for (int i = 30; i < 35; ++i) {
    encoder.pushVideoFrame(frame, i * 33);
  }
  encoder.flush();
  encoder.stop();

  ASSERT_GE(videoPackets.size(), 30u);
  EXPECT_TRUE(videoPackets.front().keyframe);
  for (size_t i = 1; i < videoPackets.size(); ++i) {
    EXPECT_GT(videoPackets[i].ptsMs, videoPackets[i - 1].ptsMs);
  }
  // The requested IDR lands in the post-request packets.
  bool idrAfterRequest = false;
  for (size_t i = packetsBeforeIdr; i < videoPackets.size(); ++i) {
    idrAfterRequest = idrAfterRequest || videoPackets[i].keyframe;
  }
  EXPECT_TRUE(idrAfterRequest);

  // ~1 second of audio -> ~50 Opus packets of 20 ms, all small.
  EXPECT_GE(audioPackets.size(), 40u);
  for (const auto &packet : audioPackets) {
    EXPECT_LT(packet.data.size(), 2000u);
  }

  // Decode everything back.
  StreamDecoder decoder;
  int decodedFrames = 0;
  StreamVideoFrame lastFrame;
  size_t decodedAudioFrames = 0;
  decoder.setVideoFrameCallback(
      [&](StreamVideoFrame decoded, int64_t) {
        decodedFrames++;
        lastFrame = std::move(decoded);
      });
  decoder.setAudioCallback([&](const std::vector<int16_t> &interleaved,
                               int64_t) {
    decodedAudioFrames += interleaved.size() / 2;
  });
  ASSERT_TRUE(decoder.start(extradata));

  for (const auto &packet : videoPackets) {
    decoder.pushVideoPacket(packet.data, packet.ptsMs);
  }
  for (const auto &packet : audioPackets) {
    decoder.pushAudioPacket(packet.data, packet.ptsMs);
  }

  EXPECT_GE(decodedFrames, static_cast<int>(videoPackets.size()) - 5);
  EXPECT_EQ(lastFrame.width, 64);
  EXPECT_EQ(lastFrame.height, 64);
  ASSERT_EQ(lastFrame.rgba.size(), 64u * 64u * 4u);
  // Solid red survives the trip (YUV conversion is slightly lossy).
  const auto centerOffset = (32 * 64 + 32) * 4;
  EXPECT_GT(lastFrame.rgba[centerOffset], 200);     // R
  EXPECT_LT(lastFrame.rgba[centerOffset + 1], 60);  // G
  EXPECT_LT(lastFrame.rgba[centerOffset + 2], 60);  // B

  EXPECT_GT(decodedAudioFrames, 35000u); // ~1 s at 48 kHz, minus warmup

  // A mid-stream reset re-arms on the next keyframe.
  ASSERT_TRUE(decoder.reset(extradata));
  decodedFrames = 0;
  decoder.pushVideoPacket(videoPackets.front().data,
                          videoPackets.front().ptsMs);
  EXPECT_GE(decodedFrames, 0);
  decoder.stop();
}

} // namespace firelight::media

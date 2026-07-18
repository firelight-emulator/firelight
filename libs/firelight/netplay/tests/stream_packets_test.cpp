#include <firelight/netplay/stream_packets.hpp>

#include <gtest/gtest.h>

namespace firelight::netplay {

TEST(StreamPacketsTest, VideoRoundTrips) {
  VideoPacket packet{.seq = 7, .ptsMs = 1234, .keyframe = true, .configChanged = true, .payload = {1, 2, 3, 4, 5}};
  const auto bytes = encodePacket(packet);
  const auto decoded = decodeVideoPacket(bytes);
  ASSERT_TRUE(decoded.has_value());
  EXPECT_EQ(decoded->seq, 7u);
  EXPECT_EQ(decoded->ptsMs, 1234u);
  EXPECT_TRUE(decoded->keyframe);
  EXPECT_TRUE(decoded->configChanged);
  EXPECT_EQ(decoded->payload, packet.payload);
}

TEST(StreamPacketsTest, AudioRoundTrips) {
  AudioPacket packet{.seq = 99, .ptsMs = 555, .payload = {9, 8, 7}};
  const auto decoded = decodeAudioPacket(encodePacket(packet));
  ASSERT_TRUE(decoded.has_value());
  EXPECT_EQ(decoded->seq, 99u);
  EXPECT_EQ(decoded->ptsMs, 555u);
  EXPECT_EQ(decoded->payload, packet.payload);
}

TEST(StreamPacketsTest, InputCarriesRedundantFramesNewestFirst) {
  InputPacket packet;
  packet.seq = 300;
  packet.displayedPtsMs = 4200;
  packet.localPadIndex = 1;
  for (uint8_t i = 0; i < 3; ++i) {
    RetropadFrameBytes frame{};
    frame[0] = i; // distinguishable content
    packet.frames.push_back(frame);
  }

  const auto decoded = decodeInputPacket(encodePacket(packet));
  ASSERT_TRUE(decoded.has_value());
  EXPECT_EQ(decoded->seq, 300u);
  EXPECT_EQ(decoded->displayedPtsMs, 4200u);
  EXPECT_EQ(decoded->localPadIndex, 1);
  ASSERT_EQ(decoded->frames.size(), 3u);
  EXPECT_EQ(decoded->frames[0][0], 0);
  EXPECT_EQ(decoded->frames[2][0], 2);
}

TEST(StreamPacketsTest, TruncatedPacketsAreRejected) {
  VideoPacket video{.payload = {1, 2, 3}};
  auto bytes = encodePacket(video);
  bytes.resize(bytes.size() - 1);
  EXPECT_FALSE(decodeVideoPacket(bytes).has_value());

  EXPECT_FALSE(decodeVideoPacket(std::vector<uint8_t>{1, 2}).has_value());
  EXPECT_FALSE(decodeAudioPacket(std::vector<uint8_t>{1}).has_value());
  EXPECT_FALSE(decodeInputPacket(std::vector<uint8_t>{1, 2, 3}).has_value());

  InputPacket input;
  input.frames.resize(2);
  auto inputBytes = encodePacket(input);
  inputBytes.resize(inputBytes.size() - RETROPAD_FRAME_BYTES);
  EXPECT_FALSE(decodeInputPacket(inputBytes).has_value());
}

} // namespace firelight::netplay

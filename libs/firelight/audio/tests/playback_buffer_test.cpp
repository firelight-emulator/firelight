// TODO: NEEDS REVIEW
#include <firelight/audio/playback_buffer.hpp>

#include <gtest/gtest.h>
#include <thread>
#include <vector>

namespace firelight::audio {

namespace {
// Frames numbered from 1: the left sample carries the low half, the right the high half, so a
// frame of two zeros is only ever silence
std::vector<int16_t> numbered(const uint32_t from, const size_t count) {
  std::vector<int16_t> frames;

  for (size_t index = 0; index < count; ++index) {
    const auto sequence = from + static_cast<uint32_t>(index);
    frames.push_back(static_cast<int16_t>(sequence & 0xffff));
    frames.push_back(static_cast<int16_t>(sequence >> 16));
  }

  return frames;
}

uint32_t sequenceOf(const std::vector<int16_t> &frames, const size_t frame) {
  return static_cast<uint16_t>(frames[frame * 2]) |
         (static_cast<uint32_t>(static_cast<uint16_t>(frames[frame * 2 + 1])) << 16);
}

bool isSilent(const std::vector<int16_t> &frames, const size_t frame) {
  return frames[frame * 2] == 0 && frames[frame * 2 + 1] == 0;
}
} // namespace

TEST(PlaybackBuffer, StaysSilentUntilPrimed) {
  PlaybackBuffer buffer;
  buffer.restart(1000, 750);
  const auto first = numbered(1, 500);
  EXPECT_EQ(buffer.push(first.data(), 500), 500u);

  std::vector<int16_t> out(100 * 2, -1);
  buffer.render(out.data(), 100);

  EXPECT_FALSE(buffer.isPlaying());
  EXPECT_TRUE(isSilent(out, 0));
  EXPECT_EQ(buffer.getSizeFrames(), 500u);
  EXPECT_DOUBLE_EQ(buffer.getOccupancy(), -1.0);

  const auto second = numbered(501, 300);
  buffer.push(second.data(), 300);
  buffer.render(out.data(), 100);

  EXPECT_TRUE(buffer.isPlaying());
  EXPECT_EQ(sequenceOf(out, 0), 1u);
  EXPECT_EQ(sequenceOf(out, 99), 100u);
  EXPECT_EQ(buffer.getSizeFrames(), 700u);
  EXPECT_DOUBLE_EQ(buffer.getOccupancy(), 0.7);
}

TEST(PlaybackBuffer, PausedRendersSilenceAndKeepsWhatItHas) {
  PlaybackBuffer buffer;
  buffer.restart(1000, 0);
  const auto frames = numbered(1, 200);
  buffer.push(frames.data(), 200);

  buffer.setPaused(true);
  std::vector<int16_t> out(50 * 2, -1);
  buffer.render(out.data(), 50);

  EXPECT_TRUE(isSilent(out, 49));
  EXPECT_EQ(buffer.getSizeFrames(), 200u);
  EXPECT_EQ(buffer.getUnderrunFrames(), 0u);

  buffer.setPaused(false);
  buffer.render(out.data(), 50);
  EXPECT_EQ(sequenceOf(out, 0), 1u);
}

TEST(PlaybackBuffer, UnderrunPadsSilenceAndCounts) {
  PlaybackBuffer buffer;
  buffer.restart(1000, 0);
  const auto frames = numbered(1, 50);
  buffer.push(frames.data(), 50);

  std::vector<int16_t> out(80 * 2, -1);
  buffer.render(out.data(), 80);

  EXPECT_EQ(sequenceOf(out, 49), 50u);
  EXPECT_TRUE(isSilent(out, 50));
  EXPECT_TRUE(isSilent(out, 79));
  EXPECT_EQ(buffer.getUnderrunFrames(), 30u);
}

TEST(PlaybackBuffer, DropsWhatIsPastTheCapacityAndCounts) {
  PlaybackBuffer buffer;
  buffer.restart(100, 0);
  const auto frames = numbered(1, 150);

  EXPECT_EQ(buffer.push(frames.data(), 150), 100u);
  EXPECT_EQ(buffer.getDroppedFrames(), 50u);
  EXPECT_EQ(buffer.getSizeFrames(), 100u);

  std::vector<int16_t> out(10 * 2);
  buffer.render(out.data(), 10);
  EXPECT_DOUBLE_EQ(buffer.getOccupancy(), 0.9);
}

TEST(PlaybackBuffer, RestartDiscardsAndPrimesAgain) {
  PlaybackBuffer buffer;
  buffer.restart(1000, 0);
  const auto frames = numbered(1, 200);
  buffer.push(frames.data(), 200);
  std::vector<int16_t> out(10 * 2);
  buffer.render(out.data(), 10);
  ASSERT_TRUE(buffer.isPlaying());

  buffer.restart(500, 100);

  EXPECT_EQ(buffer.getSizeFrames(), 0u);
  EXPECT_EQ(buffer.getCapacityFrames(), 500u);
  EXPECT_FALSE(buffer.isPlaying());
}

TEST(PlaybackBuffer, CapacityIsClampedToWhatItCanHold) {
  PlaybackBuffer buffer;
  buffer.restart(PlaybackBuffer::MAX_FRAMES * 4, PlaybackBuffer::MAX_FRAMES * 3);

  EXPECT_EQ(buffer.getCapacityFrames(), PlaybackBuffer::MAX_FRAMES);
}

TEST(PlaybackBuffer, AConcurrentRendererHearsEveryFrameInOrder) {
  PlaybackBuffer buffer;
  buffer.restart(512, 256);
  constexpr uint32_t TOTAL = 50000;

  std::thread producer([&] {
    uint32_t next = 1;

    while (next <= TOTAL) {
      const auto batch = numbered(next, std::min<uint32_t>(37, TOTAL - next + 1));
      next += static_cast<uint32_t>(buffer.push(batch.data(), batch.size() / 2));
    }
  });

  uint32_t expected = 1;
  std::vector<int16_t> out(64 * 2);

  while (expected <= TOTAL) {
    buffer.render(out.data(), 64);

    for (size_t frame = 0; frame < 64; ++frame) {
      if (isSilent(out, frame)) {
        continue;
      }

      ASSERT_EQ(sequenceOf(out, frame), expected);
      expected++;
    }
  }

  producer.join();
}

} // namespace firelight::audio

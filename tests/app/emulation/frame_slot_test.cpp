#include <emulation/frame_slot.hpp>
#include <gtest/gtest.h>
#include <thread>
#include <vector>

namespace firelight::emulation {

namespace {
VideoFrame frameOf(const int width, const int height, const uint8_t fill = 0) {
  VideoFrame frame;
  frame.width = width;
  frame.height = height;
  frame.pixels.assign(static_cast<size_t>(width) * height * 4, fill);
  return frame;
}
} // namespace

TEST(FrameSlotTest, StartsEmpty) {
  const FrameSlot slot;

  EXPECT_EQ(slot.get(), nullptr);
  EXPECT_EQ(slot.getLastId(), 0u);
}

TEST(FrameSlotTest, PublishingMakesAFrameReadable) {
  FrameSlot slot;
  slot.publish(frameOf(4, 2, 7));

  const auto frame = slot.get();

  ASSERT_NE(frame, nullptr);
  EXPECT_EQ(frame->width, 4);
  EXPECT_EQ(frame->height, 2);
  EXPECT_EQ(frame->pixels.front(), 7);
  EXPECT_FALSE(frame->isNull());
}

// The invariant the whole design leans on: a consumer can always tell whether what it is holding is
// older than what it already showed
TEST(FrameSlotTest, IdsStrictlyIncrease) {
  FrameSlot slot;

  const auto first = slot.publish(frameOf(1, 1));
  const auto second = slot.publish(frameOf(1, 1));
  const auto third = slot.publish(frameOf(1, 1));

  EXPECT_EQ(first, 1u);
  EXPECT_EQ(second, 2u);
  EXPECT_EQ(third, 3u);
  EXPECT_EQ(slot.get()->id, 3u);
}

TEST(FrameSlotTest, LatestWins) {
  FrameSlot slot;
  slot.publish(frameOf(1, 1, 1));
  slot.publish(frameOf(1, 1, 2));

  EXPECT_EQ(slot.get()->pixels.front(), 2);
}

// A reader holds a snapshot: frames published while it is looking must not change what it sees
TEST(FrameSlotTest, AHeldFrameSurvivesLaterPublishes) {
  FrameSlot slot;
  slot.publish(frameOf(1, 1, 1));

  const auto held = slot.get();
  slot.publish(frameOf(1, 1, 2));
  slot.publish(frameOf(1, 1, 3));

  EXPECT_EQ(held->pixels.front(), 1);
  EXPECT_EQ(held->id, 1u);
  EXPECT_EQ(slot.get()->pixels.front(), 3);
}

// Clearing is how a state change stops a frame from the old state being shown; the consumer holds
// what it last had instead
TEST(FrameSlotTest, ClearingLeavesNothingToRead) {
  FrameSlot slot;
  slot.publish(frameOf(1, 1));

  slot.clear();

  EXPECT_EQ(slot.get(), nullptr);
}

TEST(FrameSlotTest, ClearingDoesNotRewindIds) {
  FrameSlot slot;
  slot.publish(frameOf(1, 1));
  slot.clear();

  EXPECT_EQ(slot.publish(frameOf(1, 1)), 2u);
}

TEST(FrameSlotTest, ANullFrameIsRecognisable) {
  FrameSlot slot;
  slot.publish(VideoFrame{});

  EXPECT_TRUE(slot.get()->isNull());
}

// The real arrangement: one thread publishing while another reads. Every read must see a whole
// frame, never a torn one, and ids must never go backwards
TEST(FrameSlotTest, ReadsAndPublishesFromDifferentThreads) {
  FrameSlot slot;
  slot.publish(frameOf(64, 64, 1));

  std::atomic writing{true};
  std::thread producer([&slot, &writing] {
    for (auto i = 0; i < 2000; ++i) {
      slot.publish(frameOf(64, 64, static_cast<uint8_t>(i % 256)));
    }
    writing = false;
  });

  uint64_t lastSeen = 0;
  auto reads = 0;
  while (writing) {
    if (const auto frame = slot.get()) {
      EXPECT_GE(frame->id, lastSeen);
      lastSeen = frame->id;
      // Every byte of a frame belongs to the same publish
      EXPECT_EQ(frame->pixels.front(), frame->pixels.back());
      EXPECT_EQ(frame->pixels.size(), 64u * 64u * 4u);
      reads++;
    }
  }

  producer.join();

  EXPECT_GT(reads, 0);
  EXPECT_EQ(slot.getLastId(), 2001u);
}

} // namespace firelight::emulation

#include <firelight/tas/greenzone_store.hpp>

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

namespace firelight::tas {

namespace {
// A synthetic state blob that encodes its frame index, so tests can prove the
// restored state matches the requested keyframe without any real core.
std::vector<uint8_t> blob(uint64_t frame) {
  std::vector<uint8_t> b(8);
  for (int i = 0; i < 8; ++i) {
    b[static_cast<std::size_t>(i)] = static_cast<uint8_t>((frame >> (8 * i)) & 0xFF);
  }
  return b;
}
} // namespace

TEST(GreenzoneStoreTest, ShouldCaptureFollowsStride) {
  GreenzoneStore g({/*stride=*/60, /*maxKeyframes=*/0});
  EXPECT_TRUE(g.shouldCapture(0));
  EXPECT_TRUE(g.shouldCapture(60));
  EXPECT_TRUE(g.shouldCapture(120));
  EXPECT_FALSE(g.shouldCapture(30));
  EXPECT_FALSE(g.shouldCapture(59));

  GreenzoneStore off({/*stride=*/0, /*maxKeyframes=*/0});
  EXPECT_FALSE(off.shouldCapture(0));
}

TEST(GreenzoneStoreTest, NearestAtOrBefore) {
  GreenzoneStore g;
  EXPECT_FALSE(g.nearestAtOrBefore(100).has_value()); // empty
  for (uint64_t f : {0u, 60u, 120u, 180u}) {
    g.put(f, blob(f));
  }
  EXPECT_EQ(g.nearestAtOrBefore(0), 0u);
  EXPECT_EQ(g.nearestAtOrBefore(59), 0u);
  EXPECT_EQ(g.nearestAtOrBefore(60), 60u);   // exact match counts as at-or-before
  EXPECT_EQ(g.nearestAtOrBefore(150), 120u);
  EXPECT_EQ(g.nearestAtOrBefore(1000), 180u);
}

TEST(GreenzoneStoreTest, PlanSeekRestoresNearestAndReplaysDelta) {
  GreenzoneStore g;
  EXPECT_FALSE(g.planSeek(50).has_value()); // no anchor yet
  for (uint64_t f : {0u, 60u, 120u, 180u}) {
    g.put(f, blob(f));
  }
  auto p = g.planSeek(150);
  ASSERT_TRUE(p.has_value());
  EXPECT_EQ(p->keyframe, 120u);
  EXPECT_EQ(p->replayFrames, 30u);
  // The restored blob is exactly the keyframe's state.
  const auto *s = g.state(p->keyframe);
  ASSERT_NE(s, nullptr);
  EXPECT_EQ(*s, blob(120));

  auto exact = g.planSeek(60);
  ASSERT_TRUE(exact.has_value());
  EXPECT_EQ(exact->keyframe, 60u);
  EXPECT_EQ(exact->replayFrames, 0u); // land directly on a keyframe
}

// Editing input at frame E invalidates keyframes strictly after E; keyframe[E]
// and earlier survive (state-after-F-frames convention).
TEST(GreenzoneStoreTest, InvalidateFromDropsLaterKeyframes) {
  GreenzoneStore g;
  for (uint64_t f : {0u, 60u, 120u, 180u, 240u}) {
    g.put(f, blob(f));
  }
  const auto removed = g.invalidateFrom(120);
  EXPECT_EQ(removed, 2u); // 180, 240
  EXPECT_TRUE(g.has(0));
  EXPECT_TRUE(g.has(60));
  EXPECT_TRUE(g.has(120)); // the edit boundary survives
  EXPECT_FALSE(g.has(180));
  EXPECT_FALSE(g.has(240));
}

TEST(GreenzoneStoreTest, InvalidateFromZeroKeepsOnlyAnchor) {
  GreenzoneStore g;
  for (uint64_t f : {0u, 10u, 20u}) {
    g.put(f, blob(f));
  }
  EXPECT_EQ(g.invalidateFrom(0), 2u);
  EXPECT_EQ(g.size(), 1u);
  EXPECT_TRUE(g.has(0));
}

// The budget is honored; the anchor and the most-recent keyframe are always kept,
// and the retained ladder stays sorted/spread (densest pair evicted first).
TEST(GreenzoneStoreTest, EvictionHonorsBudgetAndKeepsAnchorAndLatest) {
  GreenzoneStore g({/*stride=*/10, /*maxKeyframes=*/5});
  for (uint64_t f = 0; f <= 100; f += 10) {
    g.put(f, blob(f));
  }
  EXPECT_EQ(g.size(), 5u);
  EXPECT_TRUE(g.has(0));   // anchor never evicted
  EXPECT_TRUE(g.has(100)); // latest never evicted

  const auto idx = g.keyframeIndices();
  ASSERT_EQ(idx.size(), 5u);
  for (std::size_t i = 1; i < idx.size(); ++i) {
    EXPECT_LT(idx[i - 1], idx[i]) << "indices must stay sorted";
  }
}

TEST(GreenzoneStoreTest, DensestPairEvictedFirst) {
  GreenzoneStore g({/*stride=*/0, /*maxKeyframes=*/3});
  g.put(0, blob(0));
  g.put(100, blob(100));
  g.put(200, blob(200)); // {0,100,200} at budget
  // Add a keyframe close to 100: the {100,105} pair is densest -> 100 evicted,
  // not 200 (the anchor and the just-added 105 are protected).
  g.put(105, blob(105)); // size 4 -> evict
  EXPECT_EQ(g.size(), 3u);
  EXPECT_TRUE(g.has(0));
  EXPECT_TRUE(g.has(105));
  EXPECT_TRUE(g.has(200));
  EXPECT_FALSE(g.has(100));
}

TEST(GreenzoneStoreTest, ByteSizeTracksBlobs) {
  GreenzoneStore g;
  g.put(0, std::vector<uint8_t>(16));
  g.put(60, std::vector<uint8_t>(16));
  EXPECT_EQ(g.byteSize(), 32u);
  g.put(0, std::vector<uint8_t>(8)); // overwrite shrinks
  EXPECT_EQ(g.byteSize(), 24u);
}

} // namespace firelight::tas

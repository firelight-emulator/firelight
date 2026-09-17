// TODO: NEEDS REVIEW
#include <firelight/audio/pcm_ring.hpp>

#include <gtest/gtest.h>
#include <numeric>
#include <thread>
#include <vector>

namespace firelight::audio {

namespace {
std::vector<int16_t> counting(const int16_t from, const size_t count) {
  std::vector<int16_t> samples(count);
  std::iota(samples.begin(), samples.end(), from);
  return samples;
}
} // namespace

TEST(PcmRing, CapacityRoundsUpToAPowerOfTwo) {
  EXPECT_EQ(PcmRing(100).getCapacity(), 128u);
  EXPECT_EQ(PcmRing(128).getCapacity(), 128u);
  EXPECT_EQ(PcmRing(1).getCapacity(), 2u);
}

TEST(PcmRing, PopsWhatWasPushedInOrder) {
  PcmRing ring(8);
  const auto in = counting(1, 5);
  ASSERT_EQ(ring.push(in.data(), in.size()), 5u);
  EXPECT_EQ(ring.getSize(), 5u);

  std::vector<int16_t> out(5);
  ASSERT_EQ(ring.pop(out.data(), out.size()), 5u);
  EXPECT_EQ(out, in);
  EXPECT_EQ(ring.getSize(), 0u);
}

TEST(PcmRing, RefusesWhatDoesNotFit) {
  PcmRing ring(8);
  const auto in = counting(1, 12);
  EXPECT_EQ(ring.push(in.data(), in.size()), 8u);
  EXPECT_EQ(ring.getSize(), 8u);
  EXPECT_EQ(ring.push(in.data(), 1), 0u);
}

TEST(PcmRing, DeliversOnlyWhatIsThere) {
  PcmRing ring(8);
  const auto in = counting(1, 3);
  ring.push(in.data(), in.size());

  std::vector<int16_t> out(6, -1);
  EXPECT_EQ(ring.pop(out.data(), out.size()), 3u);
  EXPECT_EQ(out[2], 3);
  EXPECT_EQ(out[3], -1);
}

TEST(PcmRing, WrapsAroundTheEnd) {
  PcmRing ring(8);
  const auto first = counting(1, 6);
  ring.push(first.data(), first.size());
  std::vector<int16_t> out(6);
  ring.pop(out.data(), 6);

  const auto second = counting(7, 6);
  ASSERT_EQ(ring.push(second.data(), second.size()), 6u);
  ASSERT_EQ(ring.pop(out.data(), 6), 6u);
  EXPECT_EQ(out, second);
}

TEST(PcmRing, DiscardAllEmptiesIt) {
  PcmRing ring(8);
  const auto in = counting(1, 5);
  ring.push(in.data(), in.size());

  ring.discardAll();

  EXPECT_EQ(ring.getSize(), 0u);
  ASSERT_EQ(ring.push(in.data(), 2), 2u);
  std::vector<int16_t> out(2);
  ASSERT_EQ(ring.pop(out.data(), 2), 2u);
  EXPECT_EQ(out[0], 1);
}

TEST(PcmRing, AConcurrentConsumerSeesEverySampleInOrder) {
  PcmRing ring(256);
  constexpr int16_t TOTAL = 20000;

  std::thread producer([&] {
    int16_t next = 1;

    while (next <= TOTAL) {
      const auto pushed = ring.push(&next, 1);
      next = static_cast<int16_t>(next + pushed);
    }
  });

  int16_t expected = 1;
  std::vector<int16_t> out(64);

  while (expected <= TOTAL) {
    const auto got = ring.pop(out.data(), out.size());

    for (size_t index = 0; index < got; ++index) {
      ASSERT_EQ(out[index], expected);
      expected++;
    }
  }

  producer.join();
  EXPECT_EQ(ring.getSize(), 0u);
}

} // namespace firelight::audio

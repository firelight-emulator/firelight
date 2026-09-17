// TODO: NEEDS REVIEW
#include <firelight/monitoring/trace_ring.hpp>

#include <atomic>
#include <cstdint>
#include <gtest/gtest.h>
#include <thread>
#include <vector>

namespace firelight::monitoring {

namespace {
// A record whose second word is derived from the first, so a torn read is detectable
struct Record {
  uint64_t sequence = 0;
  uint64_t payload = 0;
};

constexpr size_t CAPACITY = 8;
using Ring = TraceRing<Record, CAPACITY>;

Record record(const uint64_t sequence) { return {sequence, sequence * 10}; }

void pushRange(Ring &ring, const uint64_t from, const uint64_t to) {
  for (auto sequence = from; sequence < to; ++sequence) {
    ring.push(record(sequence));
  }
}
} // namespace

TEST(TraceRing, SnapshotIsEmptyBeforeAnyPush) {
  Ring ring;
  std::vector<Record> out;

  EXPECT_EQ(ring.snapshot(out), 0u);
  EXPECT_TRUE(out.empty());
}

TEST(TraceRing, SnapshotReturnsRecordsOldestFirst) {
  Ring ring;
  pushRange(ring, 0, 5);
  std::vector<Record> out;

  EXPECT_EQ(ring.snapshot(out), 0u);
  ASSERT_EQ(out.size(), 5u);

  for (size_t index = 0; index < out.size(); ++index) {
    EXPECT_EQ(out[index].sequence, index);
    EXPECT_EQ(out[index].payload, index * 10);
  }
}

TEST(TraceRing, ReplacesOldestPastCapacity) {
  Ring ring;
  pushRange(ring, 0, CAPACITY + 5);
  std::vector<Record> out;

  // The oldest surviving slot is the one a live writer would fill next, so it is never trusted
  EXPECT_EQ(ring.snapshot(out), 0u);
  ASSERT_EQ(out.size(), CAPACITY - 1);
  EXPECT_EQ(out.front().sequence, 6u);
  EXPECT_EQ(out.back().sequence, CAPACITY + 4);
}

TEST(TraceRing, SnapshotAppendsToExistingContents) {
  Ring ring;
  pushRange(ring, 0, 3);
  std::vector<Record> out{record(99)};

  ring.snapshot(out);

  ASSERT_EQ(out.size(), 4u);
  EXPECT_EQ(out[0].sequence, 99u);
  EXPECT_EQ(out[1].sequence, 0u);
}

TEST(TraceRing, DiscardsRecordsTheWriterPassedDuringACopy) {
  Ring ring;
  pushRange(ring, 0, CAPACITY);
  std::vector<Record> out;
  const auto cursor = ring.copyLatest(out);
  ASSERT_EQ(out.size(), CAPACITY);

  pushRange(ring, CAPACITY, CAPACITY + 2);

  // Records 0 and 1 were overwritten, and the slot of record 2 is the one the writer fills next
  EXPECT_EQ(ring.discardLapped(out, cursor), 2u);
  ASSERT_EQ(out.size(), CAPACITY - 3);
  EXPECT_EQ(out.front().sequence, 3u);
  EXPECT_EQ(out.back().sequence, CAPACITY - 1);
}

TEST(TraceRing, DropsTheSlotTheWriterMayStillBeFillingWithoutCountingIt) {
  Ring ring;
  pushRange(ring, 0, CAPACITY);
  std::vector<Record> out;
  const auto cursor = ring.copyLatest(out);

  ring.push(record(CAPACITY));

  EXPECT_EQ(ring.discardLapped(out, cursor), 1u);
  ASSERT_EQ(out.size(), CAPACITY - 2);
  EXPECT_EQ(out.front().sequence, 2u);
}

TEST(TraceRing, NothingIsDiscardedWhenTheWriterStaysBelowCapacity) {
  Ring ring;
  pushRange(ring, 0, 3);
  std::vector<Record> out;
  const auto cursor = ring.copyLatest(out);

  pushRange(ring, 3, CAPACITY - 1);

  EXPECT_EQ(ring.discardLapped(out, cursor), 0u);
  EXPECT_EQ(out.size(), 3u);
}

TEST(TraceRing, DiscardsEverythingWhenLappedEntirely) {
  Ring ring;
  pushRange(ring, 0, CAPACITY);
  std::vector<Record> out;
  const auto cursor = ring.copyLatest(out);

  pushRange(ring, CAPACITY, CAPACITY * 3);

  EXPECT_EQ(ring.discardLapped(out, cursor), CAPACITY);
  EXPECT_TRUE(out.empty());
}

TEST(TraceRing, AContinuingReadDeliversEachRecordOnce) {
  Ring ring;
  std::vector<Record> out;
  pushRange(ring, 0, 3);

  auto continuation = ring.snapshotSince(0, out);
  EXPECT_EQ(out.size(), 3u);
  EXPECT_EQ(continuation.nextIndex, 3u);
  EXPECT_EQ(continuation.lost, 0u);

  pushRange(ring, 3, 5);
  out.clear();
  continuation = ring.snapshotSince(continuation.nextIndex, out);
  ASSERT_EQ(out.size(), 2u);
  EXPECT_EQ(out.front().sequence, 3u);
  EXPECT_EQ(continuation.nextIndex, 5u);

  out.clear();
  continuation = ring.snapshotSince(continuation.nextIndex, out);
  EXPECT_TRUE(out.empty());
  EXPECT_EQ(continuation.nextIndex, 5u);
}

TEST(TraceRing, AContinuingReadCountsRecordsGoneBeforeItCame) {
  Ring ring;
  std::vector<Record> out;
  pushRange(ring, 0, 2);
  auto continuation = ring.snapshotSince(0, out);

  // Twenty more into a ring of eight: everything up to record 13 is gone, record 14 is in flight
  pushRange(ring, 2, 22);
  out.clear();
  continuation = ring.snapshotSince(continuation.nextIndex, out);

  ASSERT_EQ(out.size(), CAPACITY - 1);
  EXPECT_EQ(out.front().sequence, 15u);
  EXPECT_EQ(continuation.lost, 13u);
  EXPECT_EQ(continuation.nextIndex, 22u);
}

TEST(TraceRing, ConcurrentReaderNeverSeesATornOrStaleRecord) {
  TraceRing<Record, 64> ring;
  constexpr uint64_t TOTAL = 200000;
  std::atomic<bool> done{false};

  std::thread writer([&] {
    for (uint64_t sequence = 0; sequence < TOTAL; ++sequence) {
      ring.push(record(sequence));
    }

    done.store(true);
  });

  std::vector<Record> out;
  uint64_t snapshots = 0;
  uint64_t recordsSeen = 0;

  while (!done.load()) {
    out.clear();
    ring.snapshot(out);
    ++snapshots;

    for (size_t index = 0; index < out.size(); ++index) {
      ASSERT_EQ(out[index].payload, out[index].sequence * 10);

      if (index > 0) {
        ASSERT_EQ(out[index].sequence, out[index - 1].sequence + 1);
      }
    }

    recordsSeen += out.size();
  }

  writer.join();
  EXPECT_GT(snapshots, 0u);
  EXPECT_GT(recordsSeen, 0u);
}

} // namespace firelight::monitoring

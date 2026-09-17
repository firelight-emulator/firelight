// TODO: NEEDS REVIEW
#include <firelight/monitoring/monitor.hpp>

#include <algorithm>
#include <array>
#include <atomic>
#include <gtest/gtest.h>
#include <latch>
#include <thread>
#include <vector>

namespace firelight::monitoring {

namespace {
Snapshot collectAll(const Monitor &monitor) {
  Cursor cursor;
  Snapshot snapshot;
  monitor.collect(cursor, snapshot);
  return snapshot;
}

size_t countKind(const std::vector<SpanRecord> &spans, const KindId kind) {
  return static_cast<size_t>(
      std::count_if(spans.begin(), spans.end(), [kind](const SpanRecord &record) { return record.kind == kind; }));
}
} // namespace

TEST(Monitor, RecordsNothingWhileOff) {
  Monitor monitor;
  const auto span = monitor.span("work", "some work");
  const auto marker = monitor.marker("tick", "a tick");
  const auto series = monitor.series("level", "a level");

  const auto start = span.begin();
  span.end(start);
  marker.mark();
  series.record(1.0);

  EXPECT_EQ(start, 0);
  const auto snapshot = collectAll(monitor);
  EXPECT_TRUE(snapshot.spans.empty());
  EXPECT_TRUE(snapshot.samples.empty());
  EXPECT_FALSE(monitor.isRecording());
}

TEST(Monitor, SpanSurvivesARoundTrip) {
  Monitor monitor;
  monitor.setRecording(true);
  const auto span = monitor.span("work", "some work");

  const auto start = span.begin();
  span.end(start);

  const auto snapshot = collectAll(monitor);
  ASSERT_EQ(snapshot.spans.size(), 1u);
  EXPECT_EQ(snapshot.spans[0].kind, span.getKind());
  EXPECT_EQ(snapshot.spans[0].startNs, start);
  EXPECT_GE(snapshot.spans[0].endNs, start);
  EXPECT_EQ(snapshot.spans[0].depth, 0);
}

TEST(Monitor, NestedScopedSpansStampTheirDepth) {
  Monitor monitor;
  monitor.setRecording(true);
  const auto outer = monitor.span("outer", "");
  const auto inner = monitor.span("inner", "");
  const auto innermost = monitor.span("innermost", "");

  {
    ScopedSpan a(outer);
    {
      ScopedSpan b(inner);
      {
        ScopedSpan c(innermost);
      }
    }
    {
      ScopedSpan d(inner);
    }
  }

  const auto snapshot = collectAll(monitor);
  ASSERT_EQ(snapshot.spans.size(), 4u);

  for (const auto &record : snapshot.spans) {
    if (record.kind == outer.getKind()) {
      EXPECT_EQ(record.depth, 0);
    } else if (record.kind == inner.getKind()) {
      EXPECT_EQ(record.depth, 1);
    } else {
      EXPECT_EQ(record.depth, 2);
    }
  }
}

TEST(Monitor, MarkerIsZeroLength) {
  Monitor monitor;
  monitor.setRecording(true);
  const auto marker = monitor.marker("present", "");

  marker.mark();

  const auto snapshot = collectAll(monitor);
  ASSERT_EQ(snapshot.spans.size(), 1u);
  EXPECT_EQ(snapshot.spans[0].startNs, snapshot.spans[0].endNs);
  EXPECT_EQ(snapshot.spans[0].kind, marker.getKind());
}

TEST(Monitor, SeriesSampleSurvivesARoundTrip) {
  Monitor monitor;
  monitor.setRecording(true);
  const auto series = monitor.series("audio_buffer", "");

  series.record(0.75);

  const auto snapshot = collectAll(monitor);
  ASSERT_EQ(snapshot.samples.size(), 1u);
  EXPECT_DOUBLE_EQ(snapshot.samples[0].value, 0.75);
  EXPECT_EQ(snapshot.samples[0].kind, series.getKind());
  EXPECT_GT(snapshot.samples[0].atNs, 0);
}

TEST(Monitor, SameNameReturnsTheSameKind) {
  Monitor monitor;

  const auto first = monitor.span("run_frame", "first");
  const auto second = monitor.span("run_frame", "second");
  const auto other = monitor.span("retro_run", "");

  EXPECT_EQ(first.getKind(), second.getKind());
  EXPECT_NE(first.getKind(), other.getKind());
  const auto kinds = monitor.getKinds();
  ASSERT_EQ(kinds.size(), 2u);
  EXPECT_EQ(kinds[0].name, "run_frame");
  EXPECT_EQ(kinds[0].summary, "first");
  EXPECT_EQ(kinds[1].name, "retro_run");
  EXPECT_EQ(kinds[1].type, KindType::Span);
}

TEST(Monitor, ACursorContinuesWhereTheLastCollectStopped) {
  Monitor monitor;
  monitor.setRecording(true);
  const auto marker = monitor.marker("tick", "");
  const auto series = monitor.series("level", "");
  Cursor cursor;
  Snapshot snapshot;

  marker.mark();
  marker.mark();
  series.record(1.0);
  monitor.collect(cursor, snapshot);
  ASSERT_EQ(snapshot.spans.size(), 2u);
  ASSERT_EQ(snapshot.samples.size(), 1u);

  marker.mark();
  series.record(2.0);
  monitor.collect(cursor, snapshot);
  ASSERT_EQ(snapshot.spans.size(), 1u);
  ASSERT_EQ(snapshot.samples.size(), 1u);
  EXPECT_DOUBLE_EQ(snapshot.samples[0].value, 2.0);

  monitor.collect(cursor, snapshot);
  EXPECT_TRUE(snapshot.spans.empty());
  EXPECT_TRUE(snapshot.samples.empty());
  EXPECT_EQ(snapshot.lappedRecords, 0u);
}

TEST(Monitor, ACursorLeftBehindCountsWhatItMissed) {
  Monitor monitor;
  monitor.setRecording(true);
  const auto marker = monitor.marker("tick", "");
  Cursor cursor;
  Snapshot snapshot;

  marker.mark();
  monitor.collect(cursor, snapshot);
  ASSERT_EQ(snapshot.spans.size(), 1u);

  for (size_t count = 0; count < Monitor::RING_CAPACITY + 100; ++count) {
    marker.mark();
  }

  monitor.collect(cursor, snapshot);
  EXPECT_EQ(snapshot.spans.size(), Monitor::RING_CAPACITY - 1);
  EXPECT_EQ(snapshot.lappedRecords, 101u) << "the hundred overwritten plus the slot in flight";
}

TEST(Monitor, LastDurationFollowsTheLatestEnd) {
  Monitor monitor;
  monitor.setRecording(true);
  const auto span = monitor.span("work", "");

  EXPECT_EQ(monitor.getLastDurationNs(span), 0);

  const auto start = span.begin();
  span.end(start);

  EXPECT_GE(monitor.getLastDurationNs(span), 0);
  const auto snapshot = collectAll(monitor);
  ASSERT_EQ(snapshot.spans.size(), 1u);
  EXPECT_EQ(monitor.getLastDurationNs(span), snapshot.spans[0].endNs - snapshot.spans[0].startNs);
}

TEST(Monitor, TogglingOffBetweenBeginAndEndWritesNothingAndKeepsDepthBalanced) {
  Monitor monitor;
  monitor.setRecording(true);
  const auto span = monitor.span("work", "");

  const auto start = span.begin();
  monitor.setRecording(false);
  span.end(start);
  EXPECT_TRUE(collectAll(monitor).spans.empty());

  monitor.setRecording(true);
  const auto again = span.begin();
  span.end(again);

  const auto snapshot = collectAll(monitor);
  ASSERT_EQ(snapshot.spans.size(), 1u);
  EXPECT_EQ(snapshot.spans[0].depth, 0);
}

TEST(Monitor, ConcurrentWritersLoseNothingBelowCapacity) {
  Monitor monitor;
  monitor.setRecording(true);
  const auto span = monitor.span("work", "");
  constexpr int THREADS = 4;
  constexpr int PER_THREAD = 1000;

  std::vector<std::thread> threads;

  for (auto index = 0; index < THREADS; ++index) {
    threads.emplace_back([&] {
      for (auto count = 0; count < PER_THREAD; ++count) {
        ScopedSpan scoped(span);
      }
    });
  }

  for (auto &thread : threads) {
    thread.join();
  }

  const auto snapshot = collectAll(monitor);
  EXPECT_EQ(snapshot.spans.size(), static_cast<size_t>(THREADS * PER_THREAD));
  EXPECT_EQ(snapshot.lappedRecords, 0u);
  EXPECT_EQ(snapshot.droppedRecords, 0u);
  EXPECT_TRUE(std::is_sorted(snapshot.spans.begin(), snapshot.spans.end(),
                             [](const SpanRecord &a, const SpanRecord &b) { return a.startNs < b.startNs; }));

  std::array<int, Monitor::MAX_THREADS> perSlot{};

  for (const auto &record : snapshot.spans) {
    ASSERT_LT(record.threadSlot, Monitor::MAX_THREADS);
    perSlot[record.threadSlot]++;
  }

  EXPECT_EQ(std::count(perSlot.begin(), perSlot.end(), PER_THREAD), THREADS);
}

TEST(Monitor, AThreadThatExitsGivesItsSlotBack) {
  Monitor monitor;
  monitor.setRecording(true);
  const auto marker = monitor.marker("tick", "");

  for (auto round = 0; round < Monitor::MAX_THREADS * 2; ++round) {
    std::thread([&] { marker.mark(); }).join();
  }

  const auto snapshot = collectAll(monitor);
  EXPECT_EQ(snapshot.spans.size(), static_cast<size_t>(Monitor::MAX_THREADS * 2));
  EXPECT_EQ(snapshot.droppedRecords, 0u);
}

TEST(Monitor, WritesAreDroppedAndCountedWhenEverySlotIsHeld) {
  Monitor monitor;
  monitor.setRecording(true);
  const auto marker = monitor.marker("tick", "");
  std::latch allClaimed(Monitor::MAX_THREADS);
  std::atomic<bool> release{false};
  std::vector<std::thread> holders;

  for (auto index = 0; index < Monitor::MAX_THREADS; ++index) {
    holders.emplace_back([&] {
      marker.mark();
      allClaimed.count_down();

      while (!release.load()) {
        std::this_thread::yield();
      }
    });
  }

  allClaimed.wait();
  marker.mark();
  marker.mark();

  auto snapshot = collectAll(monitor);
  EXPECT_EQ(snapshot.spans.size(), static_cast<size_t>(Monitor::MAX_THREADS));
  EXPECT_EQ(snapshot.droppedRecords, 2u);

  release.store(true);

  for (auto &thread : holders) {
    thread.join();
  }

  marker.mark();
  snapshot = collectAll(monitor);
  EXPECT_EQ(snapshot.spans.size(), static_cast<size_t>(Monitor::MAX_THREADS + 1));
}

TEST(Monitor, TheRingKeepsTheNewestRecordsPastCapacity) {
  Monitor monitor;
  monitor.setRecording(true);
  const auto marker = monitor.marker("tick", "");

  for (size_t count = 0; count < Monitor::RING_CAPACITY + 10; ++count) {
    marker.mark();
  }

  const auto snapshot = collectAll(monitor);
  EXPECT_EQ(snapshot.spans.size(), Monitor::RING_CAPACITY - 1);
  EXPECT_EQ(snapshot.lappedRecords, 0u);
  EXPECT_EQ(countKind(snapshot.spans, marker.getKind()), Monitor::RING_CAPACITY - 1);
}

TEST(Monitor, InstanceIsTheSameObjectEveryTime) { EXPECT_EQ(&Monitor::instance(), &Monitor::instance()); }

} // namespace firelight::monitoring

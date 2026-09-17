// TODO: NEEDS REVIEW
#pragma once

#include "trace_ring.hpp"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

namespace firelight::monitoring {

class Monitor;

/** Identifies one registered span, marker or series. 0 is never a registered kind */
using KindId = uint16_t;

/** What a registered kind records */
enum class KindType : uint8_t { Span, Marker, Series };

/** What a kind was registered with */
struct KindInfo {
  KindId id = 0;
  KindType type = KindType::Span;
  std::string name;
  std::string summary;
};

/** One timed stretch of work, or an instant when startNs equals endNs */
struct SpanRecord {
  int64_t startNs = 0;
  int64_t endNs = 0;
  KindId kind = 0;
  uint8_t depth = 0;
  uint16_t threadSlot = 0;
};

/** One value of a series */
struct SampleRecord {
  int64_t atNs = 0;
  double value = 0.0;
  KindId kind = 0;
  uint16_t threadSlot = 0;
};

/** Everything one collect() found, oldest first */
struct Snapshot {
  std::vector<SpanRecord> spans;
  std::vector<SampleRecord> samples;
  /** Records the reader asked for that their writer had replaced before they were read */
  uint64_t lappedRecords = 0;
  /** Writes that found no thread slot free */
  uint64_t droppedRecords = 0;
};

/**
 * Where a reader's last collect() stopped in every ring, so the next continues exactly there.
 * Fresh, it reads whatever every ring still holds
 */
struct Cursor {
  static constexpr int SLOTS = 8;
  std::array<uint64_t, SLOTS> spans{};
  std::array<uint64_t, SLOTS> samples{};
};

/**
 * Handle for one kind of span. Copyable and safe to share across threads: the start stamp is
 * returned to the caller rather than kept here
 */
class Span {
public:
  Span() = default;

  Span(Monitor *monitor, KindId kind);

  /**
   * Stamps the start and enters one level of nesting on the calling thread
   * @return The start stamp, or 0 when nothing will be recorded
   */
  [[nodiscard]] int64_t begin() const;

  /**
   * Leaves the nesting level and writes the record; a startNs of 0 writes nothing
   */
  void end(int64_t startNs) const;

  [[nodiscard]] KindId getKind() const;

private:
  Monitor *m_monitor = nullptr;
  KindId m_kind = 0;
};

/**
 * Brackets a scope with one span, keeping the start stamp on the stack
 */
class ScopedSpan {
public:
  explicit ScopedSpan(Span span);

  ~ScopedSpan();

  ScopedSpan(const ScopedSpan &) = delete;

  ScopedSpan &operator=(const ScopedSpan &) = delete;

private:
  Span m_span;
  int64_t m_startNs = 0;
};

/**
 * Handle for one kind of instant event
 */
class Marker {
public:
  Marker() = default;

  Marker(Monitor *monitor, KindId kind);

  /**
   * Records that the event happened now
   */
  void mark() const;

  [[nodiscard]] KindId getKind() const;

private:
  Monitor *m_monitor = nullptr;
  KindId m_kind = 0;
};

/**
 * Handle for one series of values over time
 */
class Series {
public:
  Series() = default;

  Series(Monitor *monitor, KindId kind);

  /**
   * Records one value at the current time
   */
  void record(double value) const;

  [[nodiscard]] KindId getKind() const;

private:
  Monitor *m_monitor = nullptr;
  KindId m_kind = 0;
};

/**
 * Registers kinds, takes their records from any thread without locking, and hands them back as a
 * time-ordered snapshot. Records only while recording is on
 */
class Monitor {
public:
  /** How many threads may write at once */
  static constexpr int MAX_THREADS = 8;

  /** How many records of each type one thread keeps */
  static constexpr size_t RING_CAPACITY = 8192;

  /** How many kinds may be registered */
  static constexpr int MAX_KINDS = 256;

  Monitor();

  ~Monitor();

  Monitor(const Monitor &) = delete;

  Monitor &operator=(const Monitor &) = delete;

  /**
   * The one instance the application records into
   */
  static Monitor &instance();

  /**
   * @return The current time on the clock every record uses, in nanoseconds
   */
  static int64_t now();

  /**
   * Registers a span kind, or returns the existing one with the same name
   */
  Span span(std::string_view name, std::string_view summary);

  /**
   * Registers a marker kind, or returns the existing one with the same name
   */
  Marker marker(std::string_view name, std::string_view summary);

  /**
   * Registers a series kind, or returns the existing one with the same name
   */
  Series series(std::string_view name, std::string_view summary);

  /**
   * Turns recording on or off. The first time it is turned on allocates the rings
   */
  void setRecording(bool recording);

  [[nodiscard]] bool isRecording() const;

  /**
   * Replaces the contents of out with every record written since the cursor's last read, oldest
   * first, and moves the cursor on. One reader per cursor at a time
   */
  void collect(Cursor &cursor, Snapshot &out) const;

  /**
   * @return The duration of the most recently ended span of this kind, on any thread, or 0
   */
  [[nodiscard]] int64_t getLastDurationNs(Span span) const;

  /**
   * @return Every registered kind, in registration order
   */
  [[nodiscard]] std::vector<KindInfo> getKinds() const;

  /**
   * Gives a thread's slot back when the thread exits, if the monitor still exists
   */
  static void releaseThreadSlot(uint64_t monitorId, int slot);

private:
  friend class Span;
  friend class Marker;
  friend class Series;

  /** One writing thread's rings and nesting depth */
  struct ThreadRings {
    TraceRing<SpanRecord, RING_CAPACITY> spans;
    TraceRing<SampleRecord, RING_CAPACITY> samples;
    int depth = 0;
  };

  /**
   * Adds a kind under the registry lock, or finds the one already using the name
   */
  KindId registerKind(std::string_view name, std::string_view summary, KindType type);

  /**
   * @return The calling thread's rings, claiming a slot on first use, or nullptr when none is free
   */
  ThreadRings *getThreadRings();

  /**
   * @return Which slot the calling thread holds, or -1
   */
  [[nodiscard]] int getThreadSlot() const;

  /**
   * @return Whether writes may proceed, with the rings visible when it is true
   */
  [[nodiscard]] bool isWritable() const;

  void writeSpan(ThreadRings &rings, KindId kind, int64_t startNs, int64_t endNs, uint8_t depth);

  void writeSample(ThreadRings &rings, KindId kind, double value);

  void noteDropped();

  uint64_t m_id = 0;

  mutable std::mutex m_mutex;
  std::vector<KindInfo> m_kinds;

  std::atomic<bool> m_recording{false};
  std::atomic<bool> m_ringsAllocated{false};
  std::array<std::unique_ptr<ThreadRings>, MAX_THREADS> m_rings;
  std::array<std::atomic<bool>, MAX_THREADS> m_slotClaimed{};
  std::array<std::atomic<int64_t>, MAX_KINDS> m_lastDurationNs{};
  std::atomic<uint64_t> m_droppedRecords{0};
};

} // namespace firelight::monitoring

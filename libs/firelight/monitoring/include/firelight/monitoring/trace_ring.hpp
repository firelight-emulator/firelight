// TODO: NEEDS REVIEW
#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <vector>

namespace firelight::monitoring {

/**
 * A fixed-size ring that one thread writes and one other thread reads. The writer never waits and
 * never allocates; the newest record replaces the oldest
 */
template <typename T, size_t N> class TraceRing {
  static_assert(std::is_trivially_copyable_v<T>);
  static_assert(sizeof(T) % sizeof(uint64_t) == 0);
  static_assert(N > 0);

public:
  static constexpr size_t CAPACITY = N;

  /** Which record indices one copy covered */
  struct Cursor {
    uint64_t begin = 0;
    uint64_t end = 0;
  };

  TraceRing() = default;

  TraceRing(const TraceRing &) = delete;

  TraceRing &operator=(const TraceRing &) = delete;

  /**
   * Appends one record, replacing the oldest when the ring is full. Writer thread only
   */
  void push(const T &record) {
    const auto index = m_writeIndex.load(std::memory_order_relaxed);
    storeSlot(index % N, record);
    m_writeIndex.store(index + 1, std::memory_order_release);
  }

  /**
   * @return How many records have ever been pushed
   */
  [[nodiscard]] uint64_t getWriteIndex() const { return m_writeIndex.load(std::memory_order_acquire); }

  /**
   * Appends the newest records to out, oldest first, minus any the writer replaced while they were
   * being copied. Reader thread only
   * @return How many records were left out
   */
  uint64_t snapshot(std::vector<T> &out) const {
    const auto cursor = copyLatest(out);
    return discardLapped(out, cursor);
  }

  /** What one continuing read delivered */
  struct Continuation {
    /** Records between the index asked for and the first one delivered, gone before they were read */
    uint64_t lost = 0;

    /** Where the next read continues from */
    uint64_t nextIndex = 0;
  };

  /**
   * Appends every record from fromIndex onward to out, oldest first, minus any the writer replaced
   * before or while they were being copied. A fromIndex of 0 means "whatever is still here" and
   * counts nothing as lost. Reader thread only
   */
  Continuation snapshotSince(const uint64_t fromIndex, std::vector<T> &out) const {
    const auto cursor = copyLatest(out, fromIndex);
    const auto sizeBefore = out.size();
    discardLapped(out, cursor);
    const auto dropped = static_cast<uint64_t>(sizeBefore - out.size());
    const auto firstDelivered = cursor.begin + dropped;
    Continuation continuation;
    continuation.nextIndex = cursor.end;
    continuation.lost = fromIndex > 0 && firstDelivered > fromIndex ? firstDelivered - fromIndex : 0;
    return continuation;
  }

  /**
   * The first half of snapshot(): copies every record from fromIndex below the write index, or
   * everything still held when fromIndex is older than that. Reader thread only
   */
  Cursor copyLatest(std::vector<T> &out, const uint64_t fromIndex = 0) const {
    Cursor cursor;
    cursor.end = m_writeIndex.load(std::memory_order_acquire);
    cursor.begin = std::max(fromIndex, cursor.end > N ? cursor.end - N : 0);
    cursor.begin = std::min(cursor.begin, cursor.end);
    out.reserve(out.size() + static_cast<size_t>(cursor.end - cursor.begin));

    for (auto index = cursor.begin; index < cursor.end; ++index) {
      out.push_back(loadSlot(static_cast<size_t>(index % N)));
    }

    return cursor;
  }

  /**
   * The second half of snapshot(): drops the oldest copied records whose slots the writer may have
   * reused since copyLatest() read the index, including the slot it may be filling right now. A
   * full ring therefore yields N - 1 records
   * @return How many of the dropped records the writer had actually overwritten
   */
  uint64_t discardLapped(std::vector<T> &out, const Cursor cursor) const {
    const auto end = m_writeIndex.load(std::memory_order_acquire);
    const auto oldestTrusted = end >= N ? end - N + 1 : 0;
    const auto oldestIntact = end > N ? end - N : 0;

    if (oldestTrusted <= cursor.begin) {
      return 0;
    }

    const auto copied = cursor.end - cursor.begin;
    const auto dropped = std::min<uint64_t>(oldestTrusted - cursor.begin, copied);
    const auto first = out.end() - static_cast<std::ptrdiff_t>(copied);
    out.erase(first, first + static_cast<std::ptrdiff_t>(dropped));

    return oldestIntact > cursor.begin ? std::min<uint64_t>(oldestIntact - cursor.begin, copied) : 0;
  }

private:
  static constexpr size_t WORDS = sizeof(T) / sizeof(uint64_t);

  /**
   * Writes one record into a slot as relaxed word stores
   */
  void storeSlot(const size_t slot, const T &record) {
    std::array<uint64_t, WORDS> words{};
    std::memcpy(words.data(), &record, sizeof(T));

    for (size_t word = 0; word < WORDS; ++word) {
      m_words[slot * WORDS + word].store(words[word], std::memory_order_relaxed);
    }
  }

  /**
   * Reads one slot back as a record
   */
  [[nodiscard]] T loadSlot(const size_t slot) const {
    std::array<uint64_t, WORDS> words{};

    for (size_t word = 0; word < WORDS; ++word) {
      words[word] = m_words[slot * WORDS + word].load(std::memory_order_relaxed);
    }

    T record{};
    std::memcpy(&record, words.data(), sizeof(T));
    return record;
  }

  std::array<std::atomic<uint64_t>, N * WORDS> m_words{};
  std::atomic<uint64_t> m_writeIndex{0};
};

} // namespace firelight::monitoring

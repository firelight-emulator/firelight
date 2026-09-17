// TODO: NEEDS REVIEW
#include <firelight/monitoring/monitor.hpp>

#include <algorithm>
#include <chrono>
#include <unordered_map>

namespace firelight::monitoring {

namespace {

/** Which slot a thread holds on one monitor */
struct SlotEntry {
  uint64_t monitorId = 0;
  int slot = -1;
};

/** How many monitors one thread may write to at once */
constexpr int MAX_MONITORS_PER_THREAD = 4;

/**
 * The slots one thread holds, given back when the thread exits
 */
struct SlotCache {
  std::array<SlotEntry, MAX_MONITORS_PER_THREAD> entries{};
  int count = 0;

  ~SlotCache() {
    for (auto index = 0; index < count; ++index) {
      Monitor::releaseThreadSlot(entries[index].monitorId, entries[index].slot);
    }
  }

  [[nodiscard]] const SlotEntry *find(const uint64_t monitorId) const {
    for (auto index = 0; index < count; ++index) {
      if (entries[index].monitorId == monitorId) {
        return &entries[index];
      }
    }

    return nullptr;
  }

  /**
   * Drops entries for monitors that no longer exist
   */
  void evict(const std::unordered_map<uint64_t, Monitor *> &alive) {
    auto kept = 0;

    for (auto index = 0; index < count; ++index) {
      if (alive.contains(entries[index].monitorId)) {
        entries[kept++] = entries[index];
      }
    }

    count = kept;
  }

  void add(const uint64_t monitorId, const int slot) { entries[count++] = {monitorId, slot}; }
};

thread_local SlotCache g_slotCache;

/**
 * The monitors that currently exist, so a thread exiting can tell whether its slot is still there
 * to give back
 */
struct Registry {
  std::mutex mutex;
  std::unordered_map<uint64_t, Monitor *> monitors;
  uint64_t nextId = 1;

  static Registry &instance() {
    static Registry registry;
    return registry;
  }
};

} // namespace

//****************
// Span
//****************

Span::Span(Monitor *monitor, const KindId kind) : m_monitor(monitor), m_kind(kind) {}

int64_t Span::begin() const {
  if (m_kind == 0 || !m_monitor || !m_monitor->isWritable()) {
    return 0;
  }

  auto *rings = m_monitor->getThreadRings();

  if (!rings) {
    m_monitor->noteDropped();
    return 0;
  }

  rings->depth++;
  return Monitor::now();
}

void Span::end(const int64_t startNs) const {
  if (startNs == 0) {
    return;
  }

  auto *rings = m_monitor->getThreadRings();

  if (!rings) {
    return;
  }

  const auto depth = --rings->depth;

  if (!m_monitor->isWritable()) {
    return;
  }

  m_monitor->writeSpan(*rings, m_kind, startNs, Monitor::now(), static_cast<uint8_t>(std::max(depth, 0)));
}

KindId Span::getKind() const { return m_kind; }

//****************
// ScopedSpan
//****************

ScopedSpan::ScopedSpan(const Span span) : m_span(span), m_startNs(span.begin()) {}

ScopedSpan::~ScopedSpan() { m_span.end(m_startNs); }

//****************
// Marker
//****************

Marker::Marker(Monitor *monitor, const KindId kind) : m_monitor(monitor), m_kind(kind) {}

void Marker::mark() const {
  if (m_kind == 0 || !m_monitor || !m_monitor->isWritable()) {
    return;
  }

  auto *rings = m_monitor->getThreadRings();

  if (!rings) {
    m_monitor->noteDropped();
    return;
  }

  const auto nowNs = Monitor::now();
  m_monitor->writeSpan(*rings, m_kind, nowNs, nowNs, 0);
}

KindId Marker::getKind() const { return m_kind; }

//****************
// Series
//****************

Series::Series(Monitor *monitor, const KindId kind) : m_monitor(monitor), m_kind(kind) {}

void Series::record(const double value) const {
  if (m_kind == 0 || !m_monitor || !m_monitor->isWritable()) {
    return;
  }

  auto *rings = m_monitor->getThreadRings();

  if (!rings) {
    m_monitor->noteDropped();
    return;
  }

  m_monitor->writeSample(*rings, m_kind, value);
}

KindId Series::getKind() const { return m_kind; }

//****************
// Monitor
//****************

Monitor::Monitor() {
  auto &registry = Registry::instance();
  std::lock_guard lock(registry.mutex);
  m_id = registry.nextId++;
  registry.monitors[m_id] = this;
}

Monitor::~Monitor() {
  auto &registry = Registry::instance();
  std::lock_guard lock(registry.mutex);
  registry.monitors.erase(m_id);
}

Monitor &Monitor::instance() {
  static Monitor monitor;
  return monitor;
}

int64_t Monitor::now() {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

Span Monitor::span(const std::string_view name, const std::string_view summary) {
  return {this, registerKind(name, summary, KindType::Span)};
}

Marker Monitor::marker(const std::string_view name, const std::string_view summary) {
  return {this, registerKind(name, summary, KindType::Marker)};
}

Series Monitor::series(const std::string_view name, const std::string_view summary) {
  return {this, registerKind(name, summary, KindType::Series)};
}

void Monitor::setRecording(const bool recording) {
  if (!recording) {
    m_recording.store(false, std::memory_order_release);
    return;
  }

  {
    std::lock_guard lock(m_mutex);

    if (!m_ringsAllocated.load(std::memory_order_acquire)) {
      for (auto &rings : m_rings) {
        rings = std::make_unique<ThreadRings>();
      }

      m_ringsAllocated.store(true, std::memory_order_release);
    }
  }

  m_recording.store(true, std::memory_order_release);
}

bool Monitor::isRecording() const { return m_recording.load(std::memory_order_acquire); }

void Monitor::collect(Cursor &cursor, Snapshot &out) const {
  static_assert(Cursor::SLOTS == MAX_THREADS);
  out.spans.clear();
  out.samples.clear();
  out.lappedRecords = 0;
  out.droppedRecords = m_droppedRecords.load(std::memory_order_relaxed);

  if (!m_ringsAllocated.load(std::memory_order_acquire)) {
    return;
  }

  for (auto slot = 0; slot < MAX_THREADS; ++slot) {
    const auto spans = m_rings[slot]->spans.snapshotSince(cursor.spans[slot], out.spans);
    cursor.spans[slot] = spans.nextIndex;
    out.lappedRecords += spans.lost;

    const auto samples = m_rings[slot]->samples.snapshotSince(cursor.samples[slot], out.samples);
    cursor.samples[slot] = samples.nextIndex;
    out.lappedRecords += samples.lost;
  }

  std::stable_sort(out.spans.begin(), out.spans.end(),
                   [](const SpanRecord &a, const SpanRecord &b) { return a.startNs < b.startNs; });
  std::stable_sort(out.samples.begin(), out.samples.end(),
                   [](const SampleRecord &a, const SampleRecord &b) { return a.atNs < b.atNs; });
}

int64_t Monitor::getLastDurationNs(const Span span) const {
  const auto kind = span.getKind();

  if (kind == 0 || kind >= MAX_KINDS) {
    return 0;
  }

  return m_lastDurationNs[kind].load(std::memory_order_relaxed);
}

std::vector<KindInfo> Monitor::getKinds() const {
  std::lock_guard lock(m_mutex);
  return m_kinds;
}

void Monitor::releaseThreadSlot(const uint64_t monitorId, const int slot) {
  auto &registry = Registry::instance();
  std::lock_guard lock(registry.mutex);
  const auto found = registry.monitors.find(monitorId);

  if (found == registry.monitors.end() || slot < 0 || slot >= MAX_THREADS) {
    return;
  }

  found->second->m_slotClaimed[slot].store(false, std::memory_order_release);
}

KindId Monitor::registerKind(const std::string_view name, const std::string_view summary, const KindType type) {
  std::lock_guard lock(m_mutex);

  for (const auto &kind : m_kinds) {
    if (kind.name == name) {
      return kind.id;
    }
  }

  if (m_kinds.size() >= static_cast<size_t>(MAX_KINDS - 1)) {
    return 0;
  }

  const auto id = static_cast<KindId>(m_kinds.size() + 1);
  m_kinds.push_back({id, type, std::string(name), std::string(summary)});
  return id;
}

Monitor::ThreadRings *Monitor::getThreadRings() {
  if (!m_ringsAllocated.load(std::memory_order_acquire)) {
    return nullptr;
  }

  auto &cache = g_slotCache;

  if (const auto *entry = cache.find(m_id)) {
    return m_rings[entry->slot].get();
  }

  if (cache.count == MAX_MONITORS_PER_THREAD) {
    auto &registry = Registry::instance();
    std::lock_guard lock(registry.mutex);
    cache.evict(registry.monitors);

    if (cache.count == MAX_MONITORS_PER_THREAD) {
      return nullptr;
    }
  }

  for (auto slot = 0; slot < MAX_THREADS; ++slot) {
    auto expected = false;

    if (m_slotClaimed[slot].compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
      cache.add(m_id, slot);
      return m_rings[slot].get();
    }
  }

  return nullptr;
}

int Monitor::getThreadSlot() const {
  const auto *entry = g_slotCache.find(m_id);
  return entry ? entry->slot : -1;
}

bool Monitor::isWritable() const { return m_recording.load(std::memory_order_acquire); }

void Monitor::writeSpan(ThreadRings &rings, const KindId kind, const int64_t startNs, const int64_t endNs,
                        const uint8_t depth) {
  SpanRecord record;
  record.startNs = startNs;
  record.endNs = endNs;
  record.kind = kind;
  record.depth = depth;
  record.threadSlot = static_cast<uint16_t>(getThreadSlot());
  rings.spans.push(record);

  if (kind < MAX_KINDS) {
    m_lastDurationNs[kind].store(endNs - startNs, std::memory_order_relaxed);
  }
}

void Monitor::writeSample(ThreadRings &rings, const KindId kind, const double value) {
  SampleRecord record;
  record.atNs = now();
  record.value = value;
  record.kind = kind;
  record.threadSlot = static_cast<uint16_t>(getThreadSlot());
  rings.samples.push(record);
}

void Monitor::noteDropped() { m_droppedRecords.fetch_add(1, std::memory_order_relaxed); }

} // namespace firelight::monitoring

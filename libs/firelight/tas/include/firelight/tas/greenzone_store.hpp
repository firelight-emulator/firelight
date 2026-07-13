#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <vector>

namespace firelight::tas {

// A sparse ladder of savestate keyframes over a movie's timeline — the "greenzone".
// It turns an O(N) replay-from-boot seek into O(stride): to reach frame F, restore
// the nearest keyframe at or before F, then re-run forward only (F - keyframeIndex)
// frames. Editing input at frame E invalidates every cached state that input E (or
// later) helped produce, so those keyframes are dropped and lazily rebuilt.
//
// Convention: keyframe[F] is the emulator state AFTER exactly F frames have been
// emulated (F inputs consumed), i.e. the state a session at frameCounter == F holds.
// keyframe[0] is the run's anchor (power-on or the movie's start savestate); it is
// always valid and never evicted.
//
// The store is parameterized only on opaque state blobs (an EmulatorInstance /
// ICore serializeState() result), so it is independent of any core, of Qt, and of
// the render thread, and is unit-tested with synthetic blobs.
class GreenzoneStore {
public:
  struct Config {
    // Capture a keyframe every `stride` frames (shouldCapture()). 0 disables the
    // periodic-capture hint (callers may still put() explicitly).
    uint64_t stride = 60;
    // Maximum keyframes retained (0 = unbounded). When exceeded, the interior
    // keyframe in the densest pair is evicted to keep the ladder evenly spread;
    // keyframe[0] and the most-recently-added keyframe are never evicted.
    std::size_t maxKeyframes = 0;
  };

  GreenzoneStore() = default; // uses Config's in-class defaults
  explicit GreenzoneStore(Config cfg) : m_cfg(cfg) {}

  [[nodiscard]] const Config &config() const { return m_cfg; }

  // Whether `frame` is on the periodic capture cadence.
  [[nodiscard]] bool shouldCapture(uint64_t frame) const {
    return m_cfg.stride != 0 && (frame % m_cfg.stride) == 0;
  }

  // Store (or overwrite) the keyframe at `frame`, then enforce the budget.
  void put(uint64_t frame, std::vector<uint8_t> state);

  [[nodiscard]] bool has(uint64_t frame) const {
    return m_keyframes.count(frame) != 0;
  }
  [[nodiscard]] std::size_t size() const { return m_keyframes.size(); }
  [[nodiscard]] bool empty() const { return m_keyframes.empty(); }

  // The nearest keyframe index at or before `frame`, if any exists.
  [[nodiscard]] std::optional<uint64_t> nearestAtOrBefore(uint64_t frame) const;

  // The stored blob for an exact keyframe index (nullptr if none is stored there).
  [[nodiscard]] const std::vector<uint8_t> *state(uint64_t frame) const;

  // Given a seek target, the plan to reach it: which keyframe to restore and how
  // many frames to re-run forward. Returns nullopt only when the store is empty
  // (no anchor yet). `replayFrames == target - keyframe`.
  struct SeekPlan {
    uint64_t keyframe = 0;
    uint64_t replayFrames = 0;
  };
  [[nodiscard]] std::optional<SeekPlan> planSeek(uint64_t target) const;

  // An edit changed the input at frame `editedFrame`. keyframe[F] is a
  // state-after-F-frames, so every keyframe with index > editedFrame is now stale
  // (the transition out of keyframe[editedFrame] changed). Drops them and returns
  // how many were removed. keyframe[editedFrame] and earlier survive.
  std::size_t invalidateFrom(uint64_t editedFrame);

  void clear();

  // Total bytes held across all keyframe blobs (for a RAM budget / diagnostics).
  [[nodiscard]] std::size_t byteSize() const;

  [[nodiscard]] std::vector<uint64_t> keyframeIndices() const;

private:
  void evictIfNeeded(uint64_t justAdded);

  Config m_cfg;
  std::map<uint64_t, std::vector<uint8_t>> m_keyframes; // frame -> state blob
};

} // namespace firelight::tas

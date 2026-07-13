#include <firelight/tas/greenzone_store.hpp>

#include <iterator>
#include <utility>

namespace firelight::tas {

void GreenzoneStore::put(uint64_t frame, std::vector<uint8_t> state) {
  m_keyframes[frame] = std::move(state);
  evictIfNeeded(frame);
}

std::optional<uint64_t>
GreenzoneStore::nearestAtOrBefore(uint64_t frame) const {
  if (m_keyframes.empty()) {
    return std::nullopt;
  }
  // upper_bound(frame) is the first key strictly greater than frame; the one
  // before it is the nearest key <= frame (if any).
  auto it = m_keyframes.upper_bound(frame);
  if (it == m_keyframes.begin()) {
    return std::nullopt; // every keyframe is after `frame`
  }
  --it;
  return it->first;
}

const std::vector<uint8_t> *GreenzoneStore::state(uint64_t frame) const {
  auto it = m_keyframes.find(frame);
  return it == m_keyframes.end() ? nullptr : &it->second;
}

std::optional<GreenzoneStore::SeekPlan>
GreenzoneStore::planSeek(uint64_t target) const {
  const auto kf = nearestAtOrBefore(target);
  if (!kf) {
    return std::nullopt;
  }
  return SeekPlan{*kf, target - *kf};
}

std::size_t GreenzoneStore::invalidateFrom(uint64_t editedFrame) {
  // Erase every keyframe strictly after the edit; keyframe[editedFrame] survives
  // because it is the pre-transition state that input[editedFrame] acts on.
  auto it = m_keyframes.upper_bound(editedFrame);
  const auto removed = static_cast<std::size_t>(std::distance(it, m_keyframes.end()));
  m_keyframes.erase(it, m_keyframes.end());
  return removed;
}

void GreenzoneStore::clear() { m_keyframes.clear(); }

std::size_t GreenzoneStore::byteSize() const {
  std::size_t total = 0;
  for (const auto &[frame, blob] : m_keyframes) {
    (void)frame;
    total += blob.size();
  }
  return total;
}

std::vector<uint64_t> GreenzoneStore::keyframeIndices() const {
  std::vector<uint64_t> out;
  out.reserve(m_keyframes.size());
  for (const auto &[frame, blob] : m_keyframes) {
    (void)blob;
    out.push_back(frame);
  }
  return out;
}

void GreenzoneStore::evictIfNeeded(uint64_t justAdded) {
  if (m_cfg.maxKeyframes == 0) {
    return;
  }
  while (m_keyframes.size() > m_cfg.maxKeyframes) {
    const uint64_t anchor = m_keyframes.begin()->first;
    // Thin the ladder where it is densest: find the smallest gap between two
    // consecutive keyframes and evict that gap's *removable* endpoint (the one
    // that is neither the anchor nor the just-added keyframe, both protected).
    // Preferring the upper endpoint keeps the anchor→first-keyframe reach short.
    auto victim = m_keyframes.end();
    uint64_t bestGap = 0;
    bool found = false;
    auto prev = m_keyframes.begin();
    auto it = std::next(prev);
    for (; it != m_keyframes.end(); ++it, ++prev) {
      const uint64_t gap = it->first - prev->first;
      auto candidate = m_keyframes.end();
      if (it->first != anchor && it->first != justAdded) {
        candidate = it; // prefer evicting the upper endpoint
      } else if (prev->first != anchor && prev->first != justAdded) {
        candidate = prev; // upper endpoint protected; evict the lower
      }
      if (candidate == m_keyframes.end()) {
        continue; // both endpoints protected — cannot close this gap
      }
      if (!found || gap < bestGap) {
        bestGap = gap;
        victim = candidate;
        found = true;
      }
    }
    if (!found) {
      break; // nothing evictable (only anchor + justAdded remain)
    }
    m_keyframes.erase(victim);
  }
}

} // namespace firelight::tas

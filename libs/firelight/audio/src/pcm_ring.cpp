// TODO: NEEDS REVIEW
#include <firelight/audio/pcm_ring.hpp>

#include <algorithm>
#include <bit>
#include <cstring>

namespace firelight::audio {

PcmRing::PcmRing(const size_t capacitySamples)
    : m_samples(std::bit_ceil(std::max<size_t>(capacitySamples, 2))), m_mask(m_samples.size() - 1) {}

size_t PcmRing::push(const int16_t *samples, const size_t count) {
  const auto written = m_written.load(std::memory_order_relaxed);
  const auto read = m_read.load(std::memory_order_acquire);
  const auto room = m_samples.size() - static_cast<size_t>(written - read);
  const auto take = std::min(count, room);

  if (take == 0) {
    return 0;
  }

  const auto start = static_cast<size_t>(written) & m_mask;
  const auto untilWrap = std::min(take, m_samples.size() - start);
  std::memcpy(m_samples.data() + start, samples, untilWrap * sizeof(int16_t));
  std::memcpy(m_samples.data(), samples + untilWrap, (take - untilWrap) * sizeof(int16_t));

  m_written.store(written + take, std::memory_order_release);
  return take;
}

size_t PcmRing::pop(int16_t *out, const size_t count) {
  const auto read = m_read.load(std::memory_order_relaxed);
  const auto written = m_written.load(std::memory_order_acquire);
  const auto take = std::min(count, static_cast<size_t>(written - read));

  if (take == 0) {
    return 0;
  }

  const auto start = static_cast<size_t>(read) & m_mask;
  const auto untilWrap = std::min(take, m_samples.size() - start);
  std::memcpy(out, m_samples.data() + start, untilWrap * sizeof(int16_t));
  std::memcpy(out + untilWrap, m_samples.data(), (take - untilWrap) * sizeof(int16_t));

  m_read.store(read + take, std::memory_order_release);
  return take;
}

void PcmRing::discardAll() { m_read.store(m_written.load(std::memory_order_acquire), std::memory_order_release); }

size_t PcmRing::getSize() const {
  // TODO
  // Read first, so the difference can never come out negative
  const auto read = m_read.load(std::memory_order_acquire);
  const auto written = m_written.load(std::memory_order_acquire);
  return static_cast<size_t>(written - read);
}

} // namespace firelight::audio

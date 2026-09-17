// TODO: NEEDS REVIEW
#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace firelight::audio {

/**
 * A fixed-size ring of samples that one thread pushes into and one other thread pops from. Neither
 * side waits or allocates
 */
class PcmRing {
public:
  /**
   * @param capacitySamples How many samples the ring holds, rounded up to a power of two
   */
  explicit PcmRing(size_t capacitySamples);

  PcmRing(const PcmRing &) = delete;

  PcmRing &operator=(const PcmRing &) = delete;

  /**
   * Appends samples. Producer thread only
   * @return How many were accepted; the rest did not fit
   */
  size_t push(const int16_t *samples, size_t count);

  /**
   * Removes the oldest samples into out. Consumer thread only
   * @return How many were delivered; the rest were not there
   */
  size_t pop(int16_t *out, size_t count);

  /**
   * Drops everything. Consumer thread only
   */
  void discardAll();

  /**
   * How many samples are held. Any thread
   */
  [[nodiscard]] size_t getSize() const;

  [[nodiscard]] size_t getCapacity() const { return m_samples.size(); }

private:
  std::vector<int16_t> m_samples;
  size_t m_mask = 0;

  alignas(64) std::atomic<uint64_t> m_written{0};
  alignas(64) std::atomic<uint64_t> m_read{0};
};

} // namespace firelight::audio

// TODO: NEEDS REVIEW
#include <firelight/audio/playback_buffer.hpp>

#include <algorithm>
#include <cstring>

namespace firelight::audio {

PlaybackBuffer::PlaybackBuffer() : m_ring(MAX_FRAMES * CHANNELS) {}

void PlaybackBuffer::restart(const size_t capacityFrames, const size_t primeFrames) {
  m_ring.discardAll();
  const auto capacity = std::clamp<size_t>(capacityFrames, 1, MAX_FRAMES);
  m_capacityFrames.store(capacity, std::memory_order_relaxed);
  m_primeFrames.store(std::min(primeFrames, capacity), std::memory_order_relaxed);
  m_priming.store(true, std::memory_order_release);
}

size_t PlaybackBuffer::push(const int16_t *frames, const size_t numFrames) {
  const auto capacity = m_capacityFrames.load(std::memory_order_relaxed);
  const auto held = getSizeFrames();
  const auto room = capacity > held ? capacity - held : 0;
  const auto take = std::min(numFrames, room);
  const auto accepted = m_ring.push(frames, take * CHANNELS) / CHANNELS;
  m_droppedFrames.fetch_add(numFrames - accepted, std::memory_order_relaxed);
  return accepted;
}

void PlaybackBuffer::render(int16_t *out, const size_t numFrames) {
  const auto silence = [out, numFrames] { std::memset(out, 0, numFrames * CHANNELS * sizeof(int16_t)); };

  if (m_paused.load(std::memory_order_relaxed)) {
    silence();
    return;
  }

  if (m_priming.load(std::memory_order_acquire)) {
    if (getSizeFrames() < m_primeFrames.load(std::memory_order_relaxed)) {
      silence();
      return;
    }

    m_priming.store(false, std::memory_order_release);
  }

  const auto delivered = m_ring.pop(out, numFrames * CHANNELS) / CHANNELS;

  if (delivered < numFrames) {
    std::memset(out + delivered * CHANNELS, 0, (numFrames - delivered) * CHANNELS * sizeof(int16_t));
    m_underrunFrames.fetch_add(numFrames - delivered, std::memory_order_relaxed);
  }
}

void PlaybackBuffer::setPaused(const bool paused) { m_paused.store(paused, std::memory_order_relaxed); }

bool PlaybackBuffer::isPaused() const { return m_paused.load(std::memory_order_relaxed); }

bool PlaybackBuffer::isPlaying() const { return !m_priming.load(std::memory_order_acquire); }

size_t PlaybackBuffer::getSizeFrames() const { return m_ring.getSize() / CHANNELS; }

size_t PlaybackBuffer::getCapacityFrames() const { return m_capacityFrames.load(std::memory_order_relaxed); }

double PlaybackBuffer::getOccupancy() const {
  if (!isPlaying()) {
    return -1.0;
  }

  return std::min(1.0, static_cast<double>(getSizeFrames()) / static_cast<double>(getCapacityFrames()));
}

uint64_t PlaybackBuffer::getUnderrunFrames() const { return m_underrunFrames.load(std::memory_order_relaxed); }

uint64_t PlaybackBuffer::getDroppedFrames() const { return m_droppedFrames.load(std::memory_order_relaxed); }

} // namespace firelight::audio

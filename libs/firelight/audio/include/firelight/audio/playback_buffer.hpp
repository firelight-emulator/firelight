// TODO: NEEDS REVIEW
#pragma once

#include "pcm_ring.hpp"

#include <atomic>
#include <cstddef>
#include <cstdint>

namespace firelight::audio {

/**
 * Stereo sound on its way from whoever produces it to the device that plays it. The producer pushes
 * frames; the audio callback renders them. Playback starts once enough has been buffered, and while
 * paused the callback hears silence and the buffer keeps what it has
 */
class PlaybackBuffer {
public:
  static constexpr size_t CHANNELS = 2;

  /** The most frames the buffer can ever hold */
  static constexpr size_t MAX_FRAMES = 65536;

  PlaybackBuffer();

  /**
   * Drops everything held, sets how much may be buffered, and holds playback until primeFrames have
   * arrived. Only while nothing is rendering
   */
  void restart(size_t capacityFrames, size_t primeFrames);

  /**
   * Appends frames, dropping those past the capacity. Producer thread only
   * @return How many were accepted
   */
  size_t push(const int16_t *frames, size_t numFrames);

  /**
   * Fills out with the next frames to play, silence where there is nothing to play. Consumer
   * thread only
   */
  void render(int16_t *out, size_t numFrames);

  void setPaused(bool paused);

  [[nodiscard]] bool isPaused() const;

  /**
   * Whether what is pushed is being played, as opposed to gathered before playback starts
   */
  [[nodiscard]] bool isPlaying() const;

  [[nodiscard]] size_t getSizeFrames() const;

  [[nodiscard]] size_t getCapacityFrames() const;

  /**
   * How full the buffer is, 0 to 1, or -1 while it is not playing
   */
  [[nodiscard]] double getOccupancy() const;

  /**
   * How many frames of silence have been rendered in place of sound that was not there in time
   */
  [[nodiscard]] uint64_t getUnderrunFrames() const;

  /**
   * How many pushed frames were dropped for lack of room
   */
  [[nodiscard]] uint64_t getDroppedFrames() const;

private:
  PcmRing m_ring;
  std::atomic<size_t> m_capacityFrames{MAX_FRAMES};
  std::atomic<size_t> m_primeFrames{0};
  std::atomic<bool> m_priming{true};
  std::atomic<bool> m_paused{false};
  std::atomic<uint64_t> m_underrunFrames{0};
  std::atomic<uint64_t> m_droppedFrames{0};
};

} // namespace firelight::audio

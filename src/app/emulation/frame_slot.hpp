// TODO: NEEDS REVIEW
#pragma once

#include <firelight/video_frame.hpp>

#include <memory>
#include <mutex>

namespace firelight::emulation {

/**
 * The latest frame the emulator produced, and the one place anything reads it from.
 *
 * Frames are published by whoever produced them and read by everyone else — the renderer, the
 * screenshot and suspend-point paths, the clip recorder, the netplay sink. Readers get a snapshot
 * that stays valid however many frames are published afterwards, so nothing has to be copied or
 * locked for the length of a read.
 *
 * Publishing stamps a strictly increasing id, which is what lets a consumer refuse to show a frame
 * older than one it already showed, and lets a test see frames it never received.
 */
class FrameSlot {
public:
  using FramePtr = std::shared_ptr<const VideoFrame>;

  /**
   * Makes frame the current one, stamping it with the next id
   * @return The id it was given
   */
  uint64_t publish(VideoFrame frame) {
    std::lock_guard lock(m_mutex);
    frame.id = ++m_lastId;
    m_frame = std::make_shared<const VideoFrame>(std::move(frame));
    return m_lastId;
  }

  /**
   * @return The current frame, or null before anything has been published
   */
  [[nodiscard]] FramePtr get() const {
    std::lock_guard lock(m_mutex);
    return m_frame;
  }

  /**
   * @return The id of the current frame, or 0 before anything has been published
   */
  [[nodiscard]] uint64_t getLastId() const {
    std::lock_guard lock(m_mutex);
    return m_lastId;
  }

  /**
   * Drops the current frame, so a consumer holds what it last showed rather than being handed
   * something that belongs to a state the emulator has left
   */
  void clear() {
    std::lock_guard lock(m_mutex);
    m_frame.reset();
  }

private:
  mutable std::mutex m_mutex;
  FramePtr m_frame;
  uint64_t m_lastId = 0;
};

} // namespace firelight::emulation

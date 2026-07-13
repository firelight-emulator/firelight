#pragma once

#include <firelight/input/input_frame.hpp>
#include <firelight/libretro/retropad_provider.hpp>
#include <firelight/tas/scripted_retropad.hpp>

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

namespace firelight::tas {

// Supplies a movie's recorded input to the core as the *authoritative* source.
// Installed via ICore::setRetropadProvider() in place of the live InputService, so
// during playback a physical controller cannot perturb the run — live-input
// suppression is simply "the movie provider is the one the CoreInputRouter holds".
//
// The provider owns a cursor (the current frame index). The owning session advances
// it exactly once per emulated frame (advance()), NOT inside getRetropadForPlayerIndex()
// — the CoreInputRouter samples every port within a single frame, so self-advancing
// there would consume several frames of input per emulated frame. Reading past the
// end yields a neutral (all-released) frame, so an over-run can never replay stale
// input. seekTo() lets a greenzone jump the cursor when restoring a keyframe.
class MovieInputProvider final : public libretro::IRetropadProvider {
public:
  MovieInputProvider() : m_pad(std::make_shared<ScriptedRetroPad>()) {}

  explicit MovieInputProvider(std::vector<input::InputFrame> frames)
      : m_pad(std::make_shared<ScriptedRetroPad>()) {
    setMovie(std::move(frames));
  }

  // The recorded single-port input log. (Multi-port lanes are a later concern; the
  // Game Boy runs, including the E4 gauntlet, are port 0 only.)
  void setMovie(std::vector<input::InputFrame> frames) {
    m_frames = std::move(frames);
    seekTo(0);
  }

  [[nodiscard]] std::size_t frameCount() const { return m_frames.size(); }
  [[nodiscard]] std::size_t cursor() const { return m_cursor; }
  [[nodiscard]] bool atEnd() const { return m_cursor >= m_frames.size(); }

  // The input this provider will present at the current cursor.
  [[nodiscard]] const input::InputFrame &currentFrame() const {
    return m_pad->frame();
  }

  // Point the cursor at frame `index` and refresh the pad to match.
  void seekTo(std::size_t index) {
    m_cursor = index;
    m_pad->setFrame(index < m_frames.size() ? m_frames[index]
                                            : input::InputFrame{});
  }

  // Advance one frame; the session calls this exactly once per emulated frame.
  void advance() { seekTo(m_cursor + 1); }

  // Which controller port this movie drives (default 0).
  void setPort(int port) { m_port = port; }
  [[nodiscard]] int port() const { return m_port; }

  std::shared_ptr<libretro::IRetroPad>
  getRetropadForPlayerIndex(int t_player) override {
    return t_player == m_port ? m_pad : nullptr;
  }

private:
  std::vector<input::InputFrame> m_frames;
  std::shared_ptr<ScriptedRetroPad> m_pad;
  std::size_t m_cursor = 0;
  int m_port = 0;
};

// The record counterpart: a decorator that forwards to the live input provider so
// the core still reads the real controller, while the session snapshots the played
// input into a movie log. captureFrame() is called once per emulated frame (after
// the core has polled) and appends the delegate pad's port-0 InputFrame — the same
// value firelight::input::captureJoypadFrame() produced for the core that frame, so
// the recorded log replays identically. platformId/controllerTypeId must match what
// the core polls with.
class RecordingInputProvider final : public libretro::IRetropadProvider {
public:
  RecordingInputProvider(libretro::IRetropadProvider *delegate, int platformId,
                         int controllerTypeId = 1)
      : m_delegate(delegate), m_platformId(platformId),
        m_controllerTypeId(controllerTypeId) {}

  std::shared_ptr<libretro::IRetroPad>
  getRetropadForPlayerIndex(int t_player) override {
    return m_delegate ? m_delegate->getRetropadForPlayerIndex(t_player) : nullptr;
  }

  // Append the current port-`m_port` input to the log. Records a neutral frame if
  // the port is empty this instant, keeping one log entry per emulated frame.
  void captureFrame() {
    input::InputFrame f{};
    if (m_delegate) {
      if (const auto pad = m_delegate->getRetropadForPlayerIndex(m_port)) {
        f = input::captureJoypadFrame(*pad, m_platformId, m_controllerTypeId);
      }
    }
    m_log.push_back(f);
  }

  void setPort(int port) { m_port = port; }
  [[nodiscard]] const std::vector<input::InputFrame> &log() const { return m_log; }
  [[nodiscard]] std::vector<input::InputFrame> takeLog() { return std::move(m_log); }
  void clear() { m_log.clear(); }

private:
  libretro::IRetropadProvider *m_delegate = nullptr;
  int m_platformId = 0;
  int m_controllerTypeId = 1;
  int m_port = 0;
  std::vector<input::InputFrame> m_log;
};

} // namespace firelight::tas

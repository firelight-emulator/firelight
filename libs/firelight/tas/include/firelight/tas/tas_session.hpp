#pragma once

#include <firelight/input/input_frame.hpp>
#include <firelight/tas/greenzone_store.hpp>
#include <firelight/tas/movie_input_provider.hpp>
#include <firelight/tas/tas_emulator.hpp>

#include <cstdint>
#include <vector>

namespace firelight::tas {

// The resident TAS controller (roadmap Phase 1). Owns the authoritative frame
// cursor, the movie, the greenzone keyframe ladder, and the movie input provider,
// and drives an ITasEmulator to play/seek/edit a movie deterministically:
//
//   * play      — advance one frame at a time (stepForward), no savestate churn;
//   * seek      — jump to any frame via the greenzone (restore nearest keyframe,
//                 replay the delta) instead of replaying from boot;
//   * edit      — change a frame's input, invalidate the now-stale greenzone tail,
//                 bump the rerecord count, and reposition to a valid state.
//
// currentFrame() counts emulated frames: it is 0 at loadMovie() and increments once
// per stepForward(). Single-threaded; the owner serializes all calls (the render
// thread in-app, the test thread in gtest), satisfying the emulator's confinement.
class TasSession {
public:
  explicit TasSession(ITasEmulator &emu, GreenzoneStore::Config gz = {});

  // Begin authoring/replaying `frames`. The emulator must already be at the movie's
  // start (power-on or its start savestate); loadMovie captures that as keyframe 0,
  // installs the movie as the authoritative input source, and rewinds the cursor.
  void loadMovie(std::vector<input::InputFrame> frames);

  [[nodiscard]] uint64_t currentFrame() const { return m_frame; }
  [[nodiscard]] std::size_t movieLength() const { return m_movie.size(); }
  [[nodiscard]] uint32_t rerecordCount() const { return m_rerecordCount; }
  [[nodiscard]] const GreenzoneStore &greenzone() const { return m_greenzone; }
  [[nodiscard]] const std::vector<input::InputFrame> &movie() const {
    return m_movie;
  }

  // Emulate exactly one frame, consuming the movie's input at the current cursor.
  void stepForward();

  // Move to `frame`, using the greenzone to avoid replaying from the start. Seeking
  // forward continues from the current position when possible; seeking backward (or
  // past a keyframe) restores the nearest keyframe at or before the target first.
  void seekTo(uint64_t frame);

  // Change the input at `frame` (an author edit / rerecord): updates the movie,
  // invalidates greenzone keyframes after `frame`, bumps the rerecord count, and —
  // if the session is currently past `frame` — repositions to `frame` so the
  // emulator no longer holds a now-stale state.
  void editFrame(uint64_t frame, const input::InputFrame &input);

private:
  void advanceOneFrame();      // the shared play/replay step
  void captureKeyframeIfDue(); // greenzone.put() on the stride cadence

  ITasEmulator &m_emu;
  std::vector<input::InputFrame> m_movie;
  MovieInputProvider m_provider;
  GreenzoneStore m_greenzone;
  uint64_t m_frame = 0;
  uint32_t m_rerecordCount = 0;
};

} // namespace firelight::tas

#include <firelight/tas/tas_session.hpp>

namespace firelight::tas {

TasSession::TasSession(ITasEmulator &emu, GreenzoneStore::Config gz)
    : m_emu(emu), m_greenzone(gz) {}

void TasSession::loadMovie(std::vector<input::InputFrame> frames) {
  m_movie = std::move(frames);
  m_provider.setMovie(m_movie); // provider cursor -> 0
  m_emu.setRetropadProvider(&m_provider);
  m_frame = 0;
  m_greenzone.clear();
  // Anchor: the emulator's current (movie-start) state, always keyframe 0.
  m_greenzone.put(0, m_emu.serializeState());
}

void TasSession::stepForward() { advanceOneFrame(); }

void TasSession::advanceOneFrame() {
  // Invariant on entry: provider cursor == m_frame. runFrame() consumes the input
  // at the cursor; then both advance together, keeping the invariant.
  m_emu.runFrame();
  m_provider.advance();
  ++m_frame;
  captureKeyframeIfDue();
}

void TasSession::captureKeyframeIfDue() {
  if (m_greenzone.shouldCapture(m_frame)) {
    m_greenzone.put(m_frame, m_emu.serializeState());
  }
}

void TasSession::seekTo(uint64_t frame) {
  if (frame == m_frame) {
    return;
  }

  // Choose the cheapest valid starting point: keep going forward from where we are
  // if the target is ahead and no earlier restore is needed; otherwise restore the
  // nearest keyframe at or before the target and replay from there.
  const auto keyframe = m_greenzone.nearestAtOrBefore(frame);
  const bool canContinueForward = frame >= m_frame &&
                                  (!keyframe || *keyframe <= m_frame);
  if (!canContinueForward) {
    // A keyframe at or before the target must exist (loadMovie pins frame 0).
    if (!keyframe) {
      return; // no anchor to seek from — nothing loaded
    }
    const auto *state = m_greenzone.state(*keyframe);
    if (!state || !m_emu.deserializeState(*state)) {
      return;
    }
    m_frame = *keyframe;
    m_provider.seekTo(m_frame);
  }

  while (m_frame < frame) {
    advanceOneFrame();
  }
}

void TasSession::editFrame(uint64_t frame, const input::InputFrame &input) {
  if (frame >= m_movie.size()) {
    return;
  }
  m_movie[frame] = input;
  m_provider.setFrameAt(frame, input); // cursor-preserving
  // Editing frame `frame` changes the transition out of it, so every cached state
  // strictly after `frame` is stale.
  m_greenzone.invalidateFrom(frame);
  ++m_rerecordCount;
  // If we were ahead of the edit, our current emulator state is now invalid; drop
  // back to the edited frame (a still-valid pre-transition state).
  if (m_frame > frame) {
    seekTo(frame);
  }
}

} // namespace firelight::tas

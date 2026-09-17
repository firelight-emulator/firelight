// TODO: NEEDS REVIEW
#include "frame_pacer.hpp"

namespace firelight::emulation {

void FramePacer::configure(const PacingContext &context) {
  std::lock_guard lock(m_mutex);
  m_rateController.configure(context);
}

void FramePacer::reset() {
  std::lock_guard lock(m_mutex);
  forgetLocked(0);
}

void FramePacer::forgetLocked(const int64_t nowNs) {
  m_rateController.reset();
  m_submitted.store(false);
  m_lastSubmitAtNs.store(0);
  m_lastProgressNs = nowNs;
}

int64_t FramePacer::noteSubmit(const int64_t nowNs) {
  const auto lastNs = m_lastSubmitAtNs.exchange(nowNs);
  m_submitted.store(true);

  {
    std::lock_guard lock(m_mutex);
    m_rateController.notePresent(nowNs);
  }

  // TODO
  // Nothing to measure against, and the first frame after a stop must not be charged the time the
  // stop took
  if (lastNs == 0) {
    return 0;
  }

  return nowNs - lastNs;
}

void FramePacer::noteFrameDuration(const int64_t durationNs) {
  std::lock_guard lock(m_mutex);
  m_rateController.noteFrameDuration(durationNs);
}

void FramePacer::notePassDuration(const int64_t durationNs) {
  std::lock_guard lock(m_mutex);
  m_rateController.notePassDuration(durationNs);
}

void FramePacer::noteWakeLateness(const int64_t lateNs) {
  std::lock_guard lock(m_mutex);
  m_rateController.noteWakeLateness(lateNs);
}

void FramePacer::setPhaseTarget(const double target) {
  std::lock_guard lock(m_mutex);
  m_rateController.setPhaseTarget(target);
}

double FramePacer::getPhaseTarget() {
  std::lock_guard lock(m_mutex);
  return m_rateController.getPhaseTarget();
}

void FramePacer::setPaused(const bool paused) { m_paused.store(paused); }

void FramePacer::setReady(const bool ready) { m_ready.store(ready); }

void FramePacer::setAudioBufferLevel(const float level) {
  std::lock_guard lock(m_mutex);
  m_rateController.setAudioBufferLevel(level);
}

FramePacer::Decision FramePacer::tick(const int64_t nowNs) {
  std::lock_guard lock(m_mutex);

  // TODO
  // A game that is stopped, or one that hasn't come up yet, owes nothing for the time it wasn't
  // running
  if (m_paused.load() || !m_ready.load()) {
    forgetLocked(nowNs);
    return {};
  }

  if (m_submitted.exchange(false)) {
    m_lastProgressNs = nowNs;
    m_stalled.store(false);
  } else if (nowNs - m_lastProgressNs > RENDER_STALL_NS) {
    // TODO
    // Nothing has reached the display for a while: either nothing asked for a draw, or the window
    // isn't being drawn at all. Ask, and run nothing until something is
    m_lastProgressNs = nowNs;
    m_stalled.store(true);
    return {.framesToRun = 0, .shouldRequestRender = true};
  }

  if (m_stalled.load()) {
    // TODO
    // Frames run in the pass that shows them, so with nothing drawing there is nobody to run one.
    // Queueing them anyway builds a backlog that lands in one lump when drawing resumes
    m_rateController.reset();
    return {};
  }

  const auto frames = m_rateController.framesDue(nowNs);

  return {.framesToRun = frames, .shouldRequestRender = frames > 0};
}

int64_t FramePacer::getNextDeadlineNs() {
  std::lock_guard lock(m_mutex);
  return m_rateController.getNextDeadlineNs();
}

SyncMode FramePacer::getResolvedMode() {
  std::lock_guard lock(m_mutex);
  return m_rateController.getResolvedMode();
}

bool FramePacer::isFollowingTheDisplay() {
  std::lock_guard lock(m_mutex);
  return m_rateController.isFollowingTheDisplay();
}

bool FramePacer::isHoldingPhase() {
  std::lock_guard lock(m_mutex);
  return m_rateController.isPhaseLocked();
}

bool FramePacer::isPhaseLocked() {
  std::lock_guard lock(m_mutex);
  return m_rateController.isPhaseLocked() && m_rateController.getPhase().isLocked();
}

double FramePacer::getEffectiveFps() {
  std::lock_guard lock(m_mutex);
  return m_rateController.getEffectiveFps();
}

double FramePacer::getAudioRatio() {
  std::lock_guard lock(m_mutex);
  return m_rateController.getAudioRatio();
}

int FramePacer::getRefreshesPerFrame() {
  std::lock_guard lock(m_mutex);
  return m_rateController.getRefreshesPerFrame();
}

int FramePacer::getHeldRefreshes() {
  std::lock_guard lock(m_mutex);
  return m_rateController.getHeldRefreshes();
}

} // namespace firelight::emulation

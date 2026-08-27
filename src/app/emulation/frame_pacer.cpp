// TODO: NEEDS REVIEW
#include "frame_pacer.hpp"

namespace firelight::emulation {

void FramePacer::configure(const PacingContext &context) {
  std::lock_guard lock(m_mutex);
  m_rateController.configure(context);

  const auto refreshesPerFrame = m_rateController.getRefreshesPerFrame();
  m_displayPeriodNs.store(context.displayHz > 0.0 ? static_cast<int64_t>(1e9 / context.displayHz) : 0);
  m_refreshCeiling.store(RefreshCounter::ceilingFor(refreshesPerFrame));
}

void FramePacer::reset() {
  std::lock_guard lock(m_mutex);
  forgetLocked(0);
}

void FramePacer::forgetLocked(const int64_t nowNs) {
  m_rateController.reset();
  m_refreshCounter.reset();
  m_submitCount.store(0);
  m_lastSubmitAtNs.store(0);
  m_lastProgressNs = nowNs;
}

int64_t FramePacer::noteSubmit(const int64_t nowNs) {
  const auto lastNs = m_lastSubmitAtNs.exchange(nowNs);
  const auto periodNs = m_displayPeriodNs.load();

  if (lastNs == 0) {
    // TODO
    // Nothing to measure against, and the first frame after a stop must not be charged the time the
    // stop took
    m_submitCount.fetch_add(1);
    return 0;
  }

  const auto gapNs = nowNs - lastNs;
  auto refreshes = 1;

  if (periodNs > 0) {
    std::lock_guard lock(m_mutex);
    refreshes = m_refreshCounter.observe(gapNs, periodNs, m_refreshCeiling.load());
  }

  m_submitCount.fetch_add(refreshes);
  return gapNs;
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
  // running, and the frames that reached the display while it wasn't are ones nobody is waiting for
  if (m_paused.load() || !m_ready.load()) {
    forgetLocked(nowNs);
    return {};
  }

  auto refreshes = m_submitCount.exchange(0);

  if (refreshes > 0) {
    m_lastProgressNs = nowNs;
    m_stalled.store(false);
  } else if (nowNs - m_lastProgressNs > RENDER_STALL_NS) {
    // TODO
    // Nothing has reached the display for a while: either nothing asked for a draw, or the window
    // isn't being drawn at all. Asking breaks the one case that is ours to break — a frame held for
    // several refreshes stops the frames that were going to make the next one due
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

  auto frames = 0;

  // Display puts frames on the ones that reached the display; everything else runs on a clock
  for (; refreshes > 0; --refreshes) {
    frames += m_rateController.framesDueOnPresent();
  }

  frames += m_rateController.framesDue(nowNs);

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

} // namespace firelight::emulation

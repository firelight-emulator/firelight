// TODO: NEEDS REVIEW
#include "emulation_loop.hpp"

#include "diagnostics/performance_stats.hpp"
#include "emulation_service.hpp"
#include "emulator_controller.hpp"
#include "emulator_instance.hpp"
#include "pace_probe.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <thread>

namespace firelight::emulation {

namespace {
constexpr int64_t BRIEF_WAIT_NS = 1000000;

int64_t steadyNowNs() { return std::chrono::steady_clock::now().time_since_epoch().count(); }
} // namespace

int64_t PrecisionLoopClock::nowNs() { return steadyNowNs(); }

void PrecisionLoopClock::waitUntil(const int64_t untilNs) {
  const monitoring::ScopedSpan waitSpan(m_waitSpan);
  const auto marginNs = m_waiter.getSpinMarginNs();
  const auto sleepUntilNs = untilNs - marginNs;
  const auto startNs = steadyNowNs();

  if (sleepUntilNs > startNs) {
    if (!m_waiter.sleepFor(sleepUntilNs - startNs)) {
      std::this_thread::sleep_for(std::chrono::nanoseconds(sleepUntilNs - startNs));
    }

    const auto overshootNs = steadyNowNs() - sleepUntilNs;
    diagnostics::PerformanceStats::instance().recordWake(marginNs, overshootNs);
    m_spinMarginSeries.record(static_cast<double>(marginNs) / 1e6);
    m_wakeOvershootSeries.record(static_cast<double>(std::max<int64_t>(overshootNs, 0)) / 1e6);
    m_waiter.noteOvershoot(overshootNs);
  }

  // The last stretch is spun rather than slept, because a sleep that overshoots costs a frame where
  // presentation follows production. A margin of 0 makes this a no-op
  while (steadyNowNs() < untilNs) {
  }
}

EmulationLoop::EmulationLoop(ILoopClock &clock, EmulationService &service, IEmulatorController &controller,
                             LoopHooks hooks)
    : m_clock(clock), m_service(service), m_controller(controller), m_hooks(std::move(hooks)) {}

void EmulationLoop::run() {
  while (!m_stopping.load()) {
    runOnce();
  }
}

void EmulationLoop::stop() { m_stopping.store(true); }

void EmulationLoop::runOnce() {
  const auto deadlineNs = m_pacer.getNextDeadlineNs();
  m_clock.waitUntil(deadlineNs > 0 ? deadlineNs : m_clock.nowNs() + BRIEF_WAIT_NS);

  if (deadlineNs > 0) {
    m_pacer.noteWakeLateness(m_clock.nowNs() - deadlineNs);
  }

  if (PaceProbe::isEnabled()) {
    PaceProbe::instance().wakes.fetch_add(1);
  }

  if (m_stopping.load()) {
    return;
  }

  const monitoring::ScopedSpan tickSpan(m_tickSpan);
  const auto instance = m_service.getCurrentEmulatorInstanceHandle().lock();

  if (!instance) {
    m_pacer.setReady(false);
    return;
  }

  if (!instance->isInitialized()) {
    if (auto *receiver = m_hooks.receiver ? m_hooks.receiver() : nullptr) {
      instance->initialize(receiver);
    }
  }

  instance->drainCommands();

  const auto ready = instance->isInitialized() && (!m_hooks.prepareForFrames || m_hooks.prepareForFrames());
  m_pacer.setPaused(m_controller.paused());
  m_pacer.setReady(ready);

  if (m_pacer.getResolvedMode() == SyncMode::Audio) {
    m_pacer.setAudioBufferLevel(instance->getAudioBufferLevel());
  }

  const auto nowNs = m_clock.nowNs();
  const auto decision = m_pacer.tick(nowNs);

  if (PaceProbe::isEnabled()) {
    PaceProbe::instance().reportIfDue(nowNs);
  }

  if (decision.shouldRequestRender && decision.framesToRun == 0 && m_hooks.requestPass) {
    m_hooks.requestPass();
  }

  const auto frames = framesForSpeed(decision.framesToRun);

  if (frames == 0) {
    return;
  }

  if (PaceProbe::isEnabled()) {
    PaceProbe::instance().ticks.fetch_add(1);
    PaceProbe::instance().framesRun.fetch_add(frames);
  }

  for (auto frame = 0; frame < frames; ++frame) {
    const auto frameStartNs = m_clock.nowNs();
    diagnostics::PerformanceStats::instance().recordFrame(m_lastFrameNs > 0 ? frameStartNs - m_lastFrameNs : 0, 1);
    m_lastFrameNs = frameStartNs;
    instance->runFrame();
    m_pacer.noteFrameDuration(m_clock.nowNs() - frameStartNs);
  }

  if (m_hooks.requestPass) {
    m_hooks.requestPass();
  }
}

int EmulationLoop::framesForSpeed(const int framesDue) {
  if (framesDue == 0) {
    return 0;
  }

  const auto multiplier = m_controller.playbackMultiplier();

  if (multiplier >= 1.0F) {
    m_ticksToSkip = 0;
    return framesDue * std::max(1, static_cast<int>(multiplier));
  }

  // TODO
  // Below normal speed, a frame is followed by ticks that run nothing: at half speed one, at a
  // quarter three
  if (m_ticksToSkip > 0) {
    m_ticksToSkip--;
    return 0;
  }

  m_ticksToSkip = std::max(0, static_cast<int>(std::lround(1.0F / multiplier)) - 1);
  return framesDue;
}

} // namespace firelight::emulation

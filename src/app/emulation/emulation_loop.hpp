// TODO: NEEDS REVIEW
#pragma once

#include "frame_pacer.hpp"
#include "precision_waiter.hpp"

#include <firelight/libretro/video_data_receiver.hpp>
#include <firelight/monitoring/monitor.hpp>

#include <atomic>
#include <cstdint>
#include <functional>

namespace firelight::emulation {

class EmulationService;
class IEmulatorController;

/**
 * Where the loop gets time and how it waits
 */
class ILoopClock {
public:
  virtual ~ILoopClock() = default;

  [[nodiscard]] virtual int64_t nowNs() = 0;

  virtual void waitUntil(int64_t untilNs) = 0;
};

/**
 * The steady clock, waited on as accurately as the host allows
 */
class PrecisionLoopClock : public ILoopClock {
public:
  [[nodiscard]] int64_t nowNs() override;

  void waitUntil(int64_t untilNs) override;

private:
  PrecisionWaiter m_waiter;

  monitoring::Span m_waitSpan =
      monitoring::Monitor::instance().span("pace_wait", "Waiting for the next frame to be due");
  monitoring::Series m_spinMarginSeries = monitoring::Monitor::instance().series(
      "spin_margin", "How much of the wait is spun rather than slept, in milliseconds");
  monitoring::Series m_wakeOvershootSeries = monitoring::Monitor::instance().series(
      "wake_overshoot", "How late the sleep returned past its own deadline, in milliseconds");
};

/**
 * What the loop needs from whoever shows the game
 */
struct LoopHooks {
  /** What the core hands frames to, or null while nothing can take them yet */
  std::function<libretro::IVideoDataReceiver *()> receiver;

  /** Whether frames may run now; a hardware-rendered core is brought up in here. Unset means yes */
  std::function<bool()> prepareForFrames;

  /** Something new is there to show */
  std::function<void()> requestPass;
};

/**
 * Runs the game: waits for the next frame to be due, brings the emulator up if it is not, does its
 * queued work, runs the frame, and goes back to waiting. Holds no thread of its own
 */
class EmulationLoop {
public:
  EmulationLoop(ILoopClock &clock, EmulationService &service, IEmulatorController &controller, LoopHooks hooks);

  /**
   * Iterates until stop() is called
   */
  void run();

  /**
   * One wait and whatever is due after it
   */
  void runOnce();

  /**
   * Ends run() after the wait it is in. Any thread
   */
  void stop();

  [[nodiscard]] FramePacer &getPacer() { return m_pacer; }

private:
  /**
   * How many frames to run for what is due, at the speed the game is being run at
   */
  [[nodiscard]] int framesForSpeed(int framesDue);

  ILoopClock &m_clock;
  EmulationService &m_service;
  IEmulatorController &m_controller;
  LoopHooks m_hooks;
  FramePacer m_pacer;
  std::atomic<bool> m_stopping = false;

  // TODO
  // Ticks still to skip before the next frame, for a speed below normal
  int m_ticksToSkip = 0;

  int64_t m_lastFrameNs = 0;

  monitoring::Span m_tickSpan =
      monitoring::Monitor::instance().span("pace_tick", "Deciding how many frames are due and running them");
};

} // namespace firelight::emulation

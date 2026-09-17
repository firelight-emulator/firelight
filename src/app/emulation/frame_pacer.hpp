// TODO: NEEDS REVIEW
#pragma once

#include "emulation_rate_controller.hpp"

#include <atomic>
#include <cstdint>
#include <mutex>

namespace firelight::emulation {

/**
 * Decides when frames are due, from a clock held to the display's phase where the mode asks for it.
 *
 * Holds no thread and reads no clock of its own: the caller drives it and passes the time in, so the
 * whole policy can be exercised without one. Submissions arrive from the thread that draws while
 * ticks come from the thread that paces, which is what the locking here is for
 */
class FramePacer {
public:
  /**
   * What a tick concluded
   */
  struct Decision {
    /** How many frames the caller should hand to the renderer, never more than one */
    int framesToRun = 0;

    /** Whether a draw should be asked for, nothing having reached the display for a while */
    bool shouldRequestRender = false;
  };

  /**
   * How long without a frame reaching the display before one is asked for
   */
  static constexpr int64_t RENDER_STALL_NS = 250000000;

  /**
   * Applies a new pacing plan, keeping whatever phase is still meaningful
   */
  void configure(const PacingContext &context);

  /**
   * Forgets the cadence and the grid, for time the player did not experience
   */
  void reset();

  /**
   * Records a frame reaching the display, returning the gap since the last one, or 0 for the first
   */
  int64_t noteSubmit(int64_t nowNs);

  /**
   * How long the frame just run took, from its tick to its picture being ready
   */
  void noteFrameDuration(int64_t durationNs);

  /**
   * How long the pass just run took to put a picture on the target. Any thread
   */
  void notePassDuration(int64_t durationNs);

  /**
   * How late past its deadline the loop woke for the tick that follows
   */
  void noteWakeLateness(int64_t lateNs);

  /**
   * Pins where in the refresh a frame is asked for, or lets it follow the measurements when negative
   */
  void setPhaseTarget(double target);

  [[nodiscard]] double getPhaseTarget();

  /**
   * Whether the game is stopped, which owes nothing for the time it is stopped for
   */
  void setPaused(bool paused);

  /**
   * Whether there is an emulator able to run a frame yet
   */
  void setReady(bool ready);

  /**
   * How full the audio sink is, for the mode that paces off it
   */
  void setAudioBufferLevel(float level);

  /**
   * Works out what is due now
   */
  [[nodiscard]] Decision tick(int64_t nowNs);

  /**
   * When the next frame is due, or 0 where the mode has no clock
   */
  [[nodiscard]] int64_t getNextDeadlineNs();

  [[nodiscard]] SyncMode getResolvedMode();
  [[nodiscard]] bool isFollowingTheDisplay();

  /**
   * @return Whether the mode holds the anchor to the display's phase at all
   */
  [[nodiscard]] bool isHoldingPhase();

  /**
   * @return Whether the phase is held and the presents have agreed on a grid to hold it to
   */
  [[nodiscard]] bool isPhaseLocked();
  [[nodiscard]] double getEffectiveFps();
  [[nodiscard]] double getAudioRatio();
  [[nodiscard]] int getRefreshesPerFrame();
  [[nodiscard]] int getHeldRefreshes();

private:
  /**
   * Clears the rate controller's cadence, under the lock
   */
  void forgetLocked(int64_t nowNs);

  std::mutex m_mutex;
  EmulationRateController m_rateController;

  std::atomic<int64_t> m_lastSubmitAtNs = 0;
  std::atomic<bool> m_submitted = false;

  std::atomic<bool> m_paused = false;
  std::atomic<bool> m_ready = false;
  std::atomic<bool> m_stalled = true;

  int64_t m_lastProgressNs = 0;
};

} // namespace firelight::emulation

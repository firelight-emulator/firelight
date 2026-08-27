// TODO: NEEDS REVIEW
#pragma once

#include "emulation_rate_controller.hpp"
#include "refresh_counter.hpp"

#include <atomic>
#include <cstdint>
#include <mutex>

namespace firelight::emulation {

/**
 * Decides when frames are due, from a clock or from the frames reaching the display.
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
    /** How many frames the caller should hand to the renderer */
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
   * Forgets what is owed and what has been counted, for time the player did not experience
   */
  void reset();

  /**
   * Records a frame reaching the display, returning the gap since the last one, or 0 for the first
   */
  int64_t noteSubmit(int64_t nowNs);

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
  [[nodiscard]] double getEffectiveFps();
  [[nodiscard]] double getAudioRatio();
  [[nodiscard]] int getRefreshesPerFrame();

private:
  /**
   * Clears the counted refreshes and the rate controller's phase, under the lock
   */
  void forgetLocked(int64_t nowNs);

  std::mutex m_mutex;
  EmulationRateController m_rateController;
  RefreshCounter m_refreshCounter;

  // TODO
  // Written by the thread that draws and taken by the thread that paces
  std::atomic<int> m_submitCount = 0;
  std::atomic<int64_t> m_lastSubmitAtNs = 0;
  std::atomic<int64_t> m_displayPeriodNs = 0;
  std::atomic<int> m_refreshCeiling = RefreshCounter::MIN_CEILING;

  std::atomic<bool> m_paused = false;
  std::atomic<bool> m_ready = false;
  std::atomic<bool> m_stalled = true;

  int64_t m_lastProgressNs = 0;
};

} // namespace firelight::emulation

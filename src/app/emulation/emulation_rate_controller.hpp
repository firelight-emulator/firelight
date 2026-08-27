// TODO: NEEDS REVIEW
#pragma once

#include <cstdint>

namespace firelight::emulation {

/**
 * How gameplay is paced.
 */
enum class SyncMode {
  /** Whichever of the modes below suits the display, the content rate and the audio device best */
  Auto,
  /** Frames run whenever the sink has room for more audio; the device is the clock, and no rate is
      involved at all */
  Audio,
  /** A wall clock at an explicit rate */
  Fixed,
  /** One frame per N display refreshes */
  Display
};

/**
 * What the controller is told about the world. Rates are in Hz.
 */
struct PacingContext {
  SyncMode mode = SyncMode::Fixed;
  /** The rate the game is meant to run at, whatever decided it */
  double contentFps = 60.0;
  /** The display's refresh rate, or 0 when it isn't known */
  double displayHz = 0.0;
  /** Whether the audio sink can be read, which is the only thing Audio mode has to pace against */
  /** Whether presentation waits for the display's refresh. When it doesn't, a present says nothing
      about when a refresh happened, and no mode that counts them can be right */
  bool presentationLocked = false;
};

/**
 * Decides when the emulator should advance a frame, and what the audio has to be resampled by for
 * the result to play at the right speed.
 *
 * It owns the policy and nothing else: no threads, no timers, no renderer, no Qt. What drives it is
 * the caller's business — a clock for Audio and Fixed, the display's present callback for Display —
 * and the same decision can therefore be tested against a synthetic sequence of timestamps.
 */
class EmulationRateController {
public:
  /**
   * How many frames may be run at once to catch up after a stall. Beyond this the debt is dropped
   * rather than paid, so a breakpoint or a long hitch doesn't come back as a burst of fast-forward
   */
  static constexpr int MAX_CATCH_UP_FRAMES = 4;

  /**
   * How full the sink has to get before Audio mode stops asking for frames
   */
  static constexpr double TARGET_BUFFER_LEVEL = 0.5;

  /**
   * How close a whole number of refreshes has to land to the content rate for the display's rate to
   * be used in its place. The game then runs at the display's rate with its audio stretched by the
   * difference, so the bound is what that stretch is worth before it is heard: 1% is about 17 cents.
   * Every rate real content reports lands inside 0.5% of a whole division — 59.94, 60.0988 and
   * 59.7275 against 60 Hz — so nothing that should match is turned away
   */
  static constexpr double DISPLAY_MATCH_TOLERANCE = 0.01;

  /**
   * Applies a new mode, content rate or display rate, keeping the phase that is still meaningful
   */
  void configure(const PacingContext &context);

  /**
   * @return What the controller was last configured with
   */
  [[nodiscard]] const PacingContext &getContext() const { return m_context; }

  /**
   * @return The mode actually in force — what Auto settled on, and otherwise the configured mode
   */
  [[nodiscard]] SyncMode getResolvedMode() const { return m_resolvedMode; }

  // TODO
  /**
   * @return Whether the rate in force came from the display rather than from the content.
   *
   * Auto is a choice between the named modes rather than a behaviour of its own, and this is which
   * one: following the display is what "sync to monitor" does, and not following it is what "native
   * timer" does
   */
  [[nodiscard]] bool isFollowingTheDisplay() const { return m_followingDisplay; }

  /**
   * Drops accumulated timing state, so the next call starts a fresh cadence. For a game being
   * loaded, unpaused, or seeked, where the gap since the last frame means nothing
   */
  void reset();

  /**
   * How many frames are owed as of nowNs, consuming the debt it reports.
   *
   * Returns 0 when the next frame isn't due yet, and always for Display, which counts refreshes.
   * Audio answers from the sink rather than the clock: nowNs means nothing to it
   */
  [[nodiscard]] int framesDue(int64_t nowNs);

  /**
   * How many frames are owed now that the display has presented a frame. This is what Display mode
   * runs on, and it is the only way a frame lands on a refresh rather than near one.
   *
   * It depends on something outside this class: renders have to be requested continuously, not only
   * when a frame was produced. Held for more than one refresh a frame otherwise stops the presents
   * that were going to make the next one due, and the mode stalls. A panel that picks its rate from
   * how often it is asked to draw — ProMotion, or any VRR display — winds itself down the same way:
   * fewer frames mean fewer requests, a lower refresh, and so fewer frames again. Measured, that
   * spiral settled at 88 Hz and 44 fps on a 120 Hz panel.
   *
   * Which is why the mode is only ever resolved to when presentationLocked says the presents being
   * counted are refreshes — the same condition that makes rendering continuously safe rather than a
   * free-running waste of a GPU
   */
  [[nodiscard]] int framesDueOnPresent();

  /**
   * When the next frame is due, for a caller that wants to wait rather than poll. 0 when the mode
   * isn't clock-driven
   */
  [[nodiscard]] int64_t getNextDeadlineNs() const;

  /**
   * Tells Audio mode how full the sink buffer is, 0..1, or a negative value when it can't be read.
   * Room in the buffer is the only thing that makes a frame due in that mode
   */
  void setAudioBufferLevel(float level);

  /**
   * @return What the emulator's audio has to be resampled by to play at the right speed. 1.0 unless
   *   Display mode is running the game at a rate the content didn't ask for
   */
  [[nodiscard]] double getAudioRatio() const { return m_audioRatio; }

  /**
   * @return The rate frames are actually being produced at, which is the content rate except where
   *   Display mode rounded it to the refresh
   */
  [[nodiscard]] double getEffectiveFps() const { return m_effectiveFps; }

  /**
   * @return How many refreshes Display mode holds each frame for, or 0 when it isn't in use
   */
  [[nodiscard]] int getRefreshesPerFrame() const { return m_refreshesPerFrame; }

private:
  /**
   * Settles which mode is in force, the rate frames will be produced at, the audio ratio that
   * follows from it, and — in Display mode — how many refreshes each frame is held for
   */
  void resolveRates();

  /**
   * @return The rate the display can hold this content at: the refresh divided by the whole number
   *   of refreshes nearest one frame. 0 when no whole number lands within DISPLAY_MATCH_TOLERANCE,
   *   which is the display saying it cannot show this content rate
   */
  [[nodiscard]] double displayLockedFps(double contentFps) const;

  /**
   * Turns owed frames into a frame count, capping the catch-up and keeping the remainder
   */
  int takeOwedFrames();

  PacingContext m_context;

  /** What Auto chose, or the configured mode as given. Never Auto */
  SyncMode m_resolvedMode = SyncMode::Fixed;

  /** Whether the rate came from the display rather than from the content */
  bool m_followingDisplay = false;
  double m_effectiveFps = 60.0;
  double m_audioRatio = 1.0;
  int m_refreshesPerFrame = 0;

  /** Frames owed but not yet run; the fractional part is the phase */
  double m_owedFrames = 0.0;
  int64_t m_lastTickNs = 0;
  int m_refreshesSinceFrame = 0;

  /** 0..1, or negative when the sink can't be read */
  double m_audioBufferLevel = -1.0;
};

} // namespace firelight::emulation

// TODO: NEEDS REVIEW
#pragma once

/**
 * Controls the audio rate compensation for a libretro core based on the fill level of the audio sink buffer
 */
class AudioRateController {
public:
  // TODO
  /**
   * How far the rate may be pulled either way, as a fraction of the output rate. Half a percent is
   * around eight cents of pitch, which is below what is heard on program material
   */
  static constexpr double MAX_CORRECTION = 0.02;

  // TODO
  /**
   * How much audio the occupancy is averaged over, as a multiple of the sink's capacity.
   *
   * Long enough to see past a core that hands over uneven batches — mupen alternates roughly 500
   * frames one frame and 30 the next, so the buffer genuinely rises and falls every other frame. That
   * is real movement, but it is not drift, and correcting for it would put a wobble on everything the
   * game plays at half the frame rate
   */
  static constexpr double SMOOTHING_BUFFERS = 12.0;

  // TODO
  /**
   * How much audio the standing part of the correction takes to walk its full range, as a multiple of
   * the sink's capacity.
   *
   * Proportional correction alone can only hold a rate mismatch by sitting off centre, far enough
   * that the error it reads produces the correction it needs — a device 0.6% out parks the buffer at
   * 65% and keeps it there. Carrying that offset separately lets the error return to zero, which puts
   * the buffer back on target and gives a large batch the room it was supposed to have. Slow enough
   * to stay well clear of the proportional loop it sits under
   */
  static constexpr double INTEGRAL_BUFFERS = 200.0;

  /**
   * Calculates the compensation for the current fill level of the audio sink buffer, as a fraction of
   * the output rate: positive stretches the audio to fill a buffer that is draining, negative drains
   * one that is filling.
   *
   * A rate rather than a sample count, because the same correction has to mean the same thing however
   * many samples the core happened to hand over. Cores do not deliver evenly — mupen alternates
   * batches of roughly 500 and 30 — and a fixed count applied to the small one is a swing of over ten
   * percent
   *
   * @param framesThisCall How much audio this call carries, in frames at the sink's rate. Sets how
   *   far the smoothing reaches, so that it covers the same span of sound however the core divides it
   */
  double computeCompensation(int usedBytes, int bufferCapacityBytes, int framesThisCall);

  /**
   * Resets the internal state of the audio rate controller, clearing any previous usage data
   */
  void reset();

private:
  // TODO
  /** Smoothed occupancy, or negative before anything has been seen */
  double m_averageUsedBytes = -1.0;

  // TODO
  /** The part of the correction that persists once the buffer is on target */
  double m_standingCorrection = 0.0;
};

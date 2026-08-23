#pragma once

/**
 * Controls the audio rate compensation for a libretro core based on the fill level of the audio sink buffer
 */
class AudioRateController {
public:
  /**
   * Calculates the compensation for the current fill level of the audio sink buffer, as a fraction of
   * the output rate: positive stretches the audio to fill a buffer that is draining, negative drains
   * one that is filling.
   *
   * A rate rather than a sample count, because the same correction has to mean the same thing however
   * many samples the core happened to hand over. Cores do not deliver evenly — mupen alternates
   * batches of roughly 500 and 30 — and a fixed count applied to the small one is a swing of over ten
   * percent
   */
  double computeCompensation(int usedBytes, int bufferCapacityBytes);

  /**
   * Resets the internal state of the audio rate controller, clearing any previous usage data
   */
  void reset();

private:
  static constexpr int WINDOW_SIZE = 10;
  int m_usageBytes[WINDOW_SIZE] = {};
  int m_index = 0;
  int m_populatedCount = 0;
  double m_previousAvgFillRatio = -1.0;
};

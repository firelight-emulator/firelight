#pragma once

// Dynamic rate control for audio: watches the output buffer's occupancy and
// returns a small resampler compensation (samples to add or drop) that steers
// the buffer toward ~50% full. It keeps a short moving average of recent
// occupancy and only intervenes when the buffer is off-target and not already
// correcting itself, which keeps playback smooth without constant pitch wobble.
class AudioRateController {
public:
  // Describe the sink's current fill (`usedBytes` of `bufferCapacityBytes`).
  // Call once per audio batch; returns the compensation delta for that batch.
  int computeCompensation(int usedBytes, int bufferCapacityBytes);

  // Clear the moving-average window (e.g. after a device change).
  void reset();

private:
  static constexpr int kWindowSize = 10;
  int m_usageBytes[kWindowSize] = {};
  int m_index = 0;
  int m_populatedCount = 0;
  double m_previousAvgFillRatio = -1.0;
};

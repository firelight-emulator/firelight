// TODO: NEEDS REVIEW
#pragma once

#include <cstdint>
#include <mutex>
#include <string>

namespace firelight::diagnostics {

/**
 * Everything the performance overlay shows, in the units it shows them in
 */
struct PerformanceSnapshot {
  // Core
  int baseWidth = 0;
  int baseHeight = 0;
  int maxWidth = 0;
  int maxHeight = 0;
  double aspectRatio = 0.0;
  double coreFps = 0.0;
  double coreSampleRate = 0.0;

  // Video
  std::string graphicsApi;
  int viewportWidth = 0;
  int viewportHeight = 0;
  int renderWidth = 0;
  int renderHeight = 0;
  double displayHz = 0.0;
  double frameRate = 0.0;
  double frameTimeMs = 0.0;
  double frameTimeDeviationPercent = 0.0;
  double submitTimeMs = 0.0;
  double submitDeviationPercent = 0.0;
  double spinMarginMs = 0.0;
  double wakeOvershootMeanMs = 0.0;
  double wakeOvershootPeakMs = 0.0;
  int64_t framesRun = 0;
  int64_t framesLost = 0;
  std::string pacingMode;
  double audioRatio = 1.0;

  // Audio
  std::string audioDevice;
  double bufferSaturationPercent = 0.0;
  double bufferDeviationPercent = 0.0;
  double closeToUnderrunPercent = 0.0;
  double closeToBlockingPercent = 0.0;
  double correctionPercent = 0.0;
  int bufferCapacityBytes = 0;
  int64_t samplesDelivered = 0;
};

/**
 * Collects what the emulator is doing while it runs, for the overlay to read.
 *
 * The producing calls happen on the render thread once per frame and once per audio batch, so they
 * do no formatting and no allocation. Reading takes a copy under the same lock, from whatever thread
 * asks
 */
class PerformanceStats {
public:
  /**
   * The one instance, since the render thread reaches this without anywhere to inject it
   */
  static PerformanceStats &instance();

  /**
   * Records what the core asked for, which changes only when a game is loaded or changes its mode
   */
  void setCoreInfo(int baseWidth, int baseHeight, int maxWidth, int maxHeight, double aspectRatio, double coreFps);

  /**
   * Records how the frame loop resolved, which changes only when pacing is reconfigured
   */
  void setPacing(const std::string &mode, double displayHz, double audioRatio);

  /**
   * Records the size being drawn and the API drawing it
   */
  void setVideo(const std::string &graphicsApi, int renderWidth, int renderHeight);

  /**
   * Records the size the drawing ends up on screen at, which is the item's rather than the core's
   */
  void setViewport(int width, int height);

  /**
   * Records one frame's wall-clock cost and how many ran in the pass
   */
  void recordFrame(int64_t frameTimeNs, int framesRun);

  /**
   * Records a frame the pacer asked for that the render pass had no room to run
   */
  void recordDroppedFrame();

  /**
   * Records the gap between one frame being handed to the display and the next.
   *
   * This is a submission, not a flip: it is taken from frameSwapped, which fires when a present is
   * queued rather than when the display shows it. Frames in flight and a buffered swapchain mean two
   * can be handed over inside one refresh, so this figure is bursty even when scanout is perfectly
   * even, and it must not be read as presentation quality
   */
  void recordSubmit(int64_t gapNs);

  /**
   * Records how far past its own deadline a sleep returned, and how much of the frame is being spun
   * to cover it.
   *
   * The two together say whether the pacing loop is waiting accurately or merely spinning over the
   * top of a wait that is not
   */
  void recordWake(int64_t marginNs, int64_t overshootNs);

  /**
   * Records which output is open and how much it holds
   */
  void setAudioDevice(const std::string &name, int capacityBytes, double coreSampleRate);

  /**
   * Records one delivery of audio: how full the sink was, what the correction is, and how much
   * sound went in
   */
  void recordAudio(double occupancy, double correction, int64_t samples);

  /**
   * Everything above, as one consistent copy
   */
  [[nodiscard]] PerformanceSnapshot snapshot() const;

  /**
   * Clears the running totals, for a game starting rather than one being reconfigured
   */
  void reset();

private:
  // TODO
  // How far back the frame-time and occupancy figures reach. Long enough that the numbers hold still
  // to be read, short enough that a hitch is still visible when it happens
  static constexpr double SMOOTHING = 0.02;

  // TODO
  // Slower for the frame figures than for the buffer, because a deviation read off the screen
  // against another emulator's has to hold still long enough to read, and a fiftieth is short
  // enough that the estimate wanders on its own
  static constexpr double FRAME_SMOOTHING = 0.004;

  // TODO
  // How near an end of the buffer counts as nearly out of audio, or nearly out of room. The same
  // thresholds the overlay's underrun and blocking figures are a running proportion of
  // TODO
  // How much of a game's opening is left out of the rate it settles at. Two seconds covers a
  // recompiler's first pass and a hardware renderer's first pipelines
  static constexpr int64_t WARMUP_FRAMES = 120;

  // TODO
  // The longest gap that still describes a frame interval. A menu, a pause or a window that went
  // away leaves one far longer, and a total that swallows it reads slow for the rest of the session
  static constexpr double MAX_FRAME_GAP_NS = 100000000.0;

  static constexpr double UNDERRUN_LEVEL = 0.25;
  static constexpr double BLOCKING_LEVEL = 0.75;

  mutable std::mutex m_mutex;
  PerformanceSnapshot m_snapshot;

  double m_totalElapsedNs = 0.0;
  int64_t m_totalFrames = 0;
  int64_t m_warmupFrames = 0;
  double m_frameTimeMeanNs = 0.0;
  double m_frameTimeVarianceNs = 0.0;
  double m_wakeOvershootMeanNs = 0.0;
  double m_submitMeanNs = 0.0;
  double m_submitVarianceNs = 0.0;
  bool m_hasSubmit = false;
  double m_occupancyMean = 0.0;
  double m_occupancyVariance = 0.0;
  double m_underrunShare = 0.0;
  double m_blockingShare = 0.0;
  bool m_hasFrameTime = false;
  bool m_hasOccupancy = false;
};

} // namespace firelight::diagnostics

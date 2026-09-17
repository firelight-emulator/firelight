// TODO: NEEDS REVIEW
#pragma once

#include <firelight/monitoring/monitor.hpp>

#include <cstdint>
#include <deque>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace firelight::diagnostics {

/**
 * The figures the readout shows, derived from the recent history
 */
struct MonitorFigures {
  double frameRate = 0.0;
  double frameTimeMs = 0.0;
  double frameTimeDeviationPercent = 0.0;
  double submitTimeMs = 0.0;
  double submitDeviationPercent = 0.0;
  double vblankToSubmitMs = 0.0;
  double vblankToWaitEndMs = 0.0;
  double vblankToFrameEndMs = 0.0;
  double frameEndSpreadMs = 0.0;
  // TODO
  /** Where in the refresh a pass takes the newest frame, measured around the refresh period */
  double vblankToFrameGrabMs = 0.0;
  double frameGrabSpreadMs = 0.0;
  /** Presents and frames run for every vertical blank seen, over the window */
  double presentsPerBlank = 0.0;
  double framesPerBlank = 0.0;
  // TODO
  /** Blank intervals with no present or no frame in them, and with two or more, over the run */
  int64_t blanksWithoutPresent = 0;
  int64_t blanksWithTwoPresents = 0;
  int64_t blanksWithoutFrame = 0;
  int64_t blanksWithTwoFrames = 0;
  double spinMarginMs = 0.0;
  double wakeOvershootMeanMs = 0.0;
  double wakeOvershootPeakMs = 0.0;
  bool phaseLocked = false;
  double refreshPeriodMs = 0.0;
  int64_t slips = 0;
  // TODO
  /** What a held clock still owes the content's rate, and how often a stall dropped time */
  double slipDebtMs = 0.0;
  int64_t timeDrops = 0;
  int64_t framesRun = 0;
  int64_t framesLost = 0;
  int64_t framesShown = 0;
  int64_t framesNotShown = 0;
  int64_t framesRepeated = 0;
  // TODO
  /** Repeated frames that landed within NEAR_SLIP_NS of a slip, either side */
  int64_t repeatsNearSlip = 0;
  int64_t framesNoPicture = 0;
  // TODO
  /** Vertical blanks, presents and render passes seen since the first frame was shown */
  int64_t blanks = 0;
  int64_t presents = 0;
  int64_t passes = 0;
  double bufferSaturationPercent = 0.0;
  double bufferDeviationPercent = 0.0;
  double closeToUnderrunPercent = 0.0;
  double closeToBlockingPercent = 0.0;
  double correctionPercent = 0.0;
  int64_t samplesDelivered = 0;
  uint64_t lappedRecords = 0;
  uint64_t droppedRecords = 0;
};

/**
 * Keeps the last stretch of a monitor's records on the reading thread, and derives the readout's
 * figures from them. One update per tick pulls only what is new
 */
class MonitorHistory {
public:
  /** How much history is kept, in seconds */
  static constexpr double KEEP_SECONDS = 30.0;

  /** How far back the smoothed figures look, in seconds */
  static constexpr double FIGURE_WINDOW_SECONDS = 2.0;

  /** Where an interval stops describing a frame and starts describing a pause, in ns */
  static constexpr int64_t MAX_FRAME_GAP_NS = 100000000;

  /** The occupancy below which the sink counts as nearly out of audio */
  static constexpr double UNDERRUN_LEVEL = 0.25;

  /** The occupancy above which the sink counts as nearly out of room */
  static constexpr double BLOCKING_LEVEL = 0.75;

  // TODO
  /** How close to a slip, either side, a repeated frame counts as near it, in ns */
  static constexpr int64_t NEAR_SLIP_NS = 100000000;

  explicit MonitorHistory(monitoring::Monitor &monitor);

  /**
   * Pulls everything recorded since the last update and drops what has aged out
   */
  void update();

  /**
   * Forgets the history and the running totals
   */
  void reset();

  /**
   * @return The kind registered under name, or 0
   */
  [[nodiscard]] monitoring::KindId getKind(std::string_view name) const;

  [[nodiscard]] const std::vector<monitoring::SpanRecord> &getSpans() const;

  [[nodiscard]] const std::vector<monitoring::SampleRecord> &getSamples() const;

  /**
   * @return When the last update happened, on the monitor's clock
   */
  [[nodiscard]] int64_t getUpdatedNs() const;

  /**
   * Derives the readout's figures from the history as it stands
   */
  [[nodiscard]] MonitorFigures getFigures() const;

private:
  /**
   * Refreshes the name-to-kind map from the monitor's registry
   */
  void refreshKinds();

  /**
   * Adds the new records' contribution to the running totals
   */
  void accumulate(const monitoring::Snapshot &snapshot);

  /**
   * Drops records older than KEEP_SECONDS before updatedNs
   */
  void prune();

  // TODO
  /**
   * Counts a repeated frame at atNs as near a slip if a recent slip is close enough, or keeps it for
   * a slip that may still come
   */
  void noteRepeat(int64_t atNs);

  // TODO
  /**
   * Counts the kept repeated frames close enough to a slip at atNs as near it, and keeps the slip for
   * repeats that may still come
   */
  void noteSlip(int64_t atNs);

  // TODO
  /**
   * Forgets kept slips and repeats too far before atNs to be near anything that arrives from now on
   */
  void forgetBefore(int64_t atNs);

  monitoring::Monitor &m_monitor;
  monitoring::Cursor m_cursor;
  monitoring::Snapshot m_scratch;
  std::vector<monitoring::SpanRecord> m_spans;
  std::vector<monitoring::SampleRecord> m_samples;
  std::unordered_map<std::string, monitoring::KindId> m_kinds;
  size_t m_kindCount = 0;
  int64_t m_updatedNs = 0;

  int64_t m_framesRun = 0;
  /** Frames and the time between them since the start, for a rate that converges */
  int64_t m_pacedFrames = 0;
  int64_t m_pacedElapsedNs = 0;
  int64_t m_lastFrameStartNs = 0;
  int64_t m_framesLost = 0;
  int64_t m_framesShown = 0;
  int64_t m_framesRepeated = 0;
  int64_t m_framesNoPicture = 0;
  int64_t m_blanks = 0;
  int64_t m_presents = 0;
  int64_t m_passes = 0;
  int64_t m_samplesDelivered = 0;
  double m_wakeOvershootPeakMs = 0.0;

  // TODO
  /** What the blank interval now open has had in it, and how the closed ones were filled */
  bool m_hasSeenBlank = false;
  int m_presentsSinceBlank = 0;
  int m_framesSinceBlank = 0;
  int64_t m_blanksWithoutPresent = 0;
  int64_t m_blanksWithTwoPresents = 0;
  int64_t m_blanksWithoutFrame = 0;
  int64_t m_blanksWithTwoFrames = 0;
  int64_t m_slips = 0;
  int64_t m_timeDrops = 0;
  uint64_t m_lappedRecords = 0;

  // TODO
  /** Recent slips, and recent repeated frames no slip has been found near yet */
  std::deque<int64_t> m_recentSlipsNs;
  std::deque<int64_t> m_unmatchedRepeatsNs;
  int64_t m_repeatsNearSlip = 0;
};

} // namespace firelight::diagnostics

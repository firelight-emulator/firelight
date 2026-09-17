// TODO: NEEDS REVIEW
#pragma once

#include "diagnostics/monitor_history.hpp"
#include "diagnostics/performance_stats.hpp"
#include "series_source.hpp"

#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariantList>
#include <map>
#include <string>

namespace firelight::gui {

/**
 * One of the monitor's series, read out of the shared history
 */
class MonitorSeriesSource final : public SeriesSource {
  Q_OBJECT

public:
  MonitorSeriesSource(const diagnostics::MonitorHistory &history, std::string name, QObject *parent);

  void copyPoints(int64_t sinceNs, std::vector<SeriesPoint> &out) const override;

  [[nodiscard]] int64_t getNowNs() const override;

private:
  const diagnostics::MonitorHistory &m_history;
  std::string m_name;
};

/**
 * Exposes the monitor to the panel that shows it: the readout's figures as numbers, the series
 * as sources, and the kinds for the legend. Recording runs while the panel is visible
 */
class QtPerformanceMonitorProxy final : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged)
  Q_PROPERTY(bool paused READ isPaused WRITE setPaused NOTIFY pausedChanged)

  Q_PROPERTY(int baseWidth READ getBaseWidth NOTIFY updated)
  Q_PROPERTY(int baseHeight READ getBaseHeight NOTIFY updated)
  Q_PROPERTY(int maxWidth READ getMaxWidth NOTIFY updated)
  Q_PROPERTY(int maxHeight READ getMaxHeight NOTIFY updated)
  Q_PROPERTY(double aspectRatio READ getAspectRatio NOTIFY updated)
  Q_PROPERTY(double coreFps READ getCoreFps NOTIFY updated)
  Q_PROPERTY(double coreSampleRate READ getCoreSampleRate NOTIFY updated)

  Q_PROPERTY(QString graphicsApi READ getGraphicsApi NOTIFY updated)
  Q_PROPERTY(int viewportWidth READ getViewportWidth NOTIFY updated)
  Q_PROPERTY(int viewportHeight READ getViewportHeight NOTIFY updated)
  Q_PROPERTY(int renderWidth READ getRenderWidth NOTIFY updated)
  Q_PROPERTY(int renderHeight READ getRenderHeight NOTIFY updated)
  Q_PROPERTY(double displayHz READ getDisplayHz NOTIFY updated)
  Q_PROPERTY(double targetFps READ getTargetFps NOTIFY updated)
  Q_PROPERTY(QString pacingMode READ getPacingMode NOTIFY updated)
  Q_PROPERTY(double audioRatio READ getAudioRatio NOTIFY updated)

  Q_PROPERTY(double frameRate READ getFrameRate NOTIFY updated)
  Q_PROPERTY(bool rateMatchesTarget READ isRateMatchingTarget NOTIFY updated)
  Q_PROPERTY(double frameTimeMs READ getFrameTimeMs NOTIFY updated)
  Q_PROPERTY(double frameTimeDeviationPercent READ getFrameTimeDeviationPercent NOTIFY updated)
  Q_PROPERTY(double submitTimeMs READ getSubmitTimeMs NOTIFY updated)
  Q_PROPERTY(double submitDeviationPercent READ getSubmitDeviationPercent NOTIFY updated)
  Q_PROPERTY(double vblankToSubmitMs READ getVblankToSubmitMs NOTIFY updated)
  Q_PROPERTY(double vblankToWaitEndMs READ getVblankToWaitEndMs NOTIFY updated)
  Q_PROPERTY(double vblankToFrameEndMs READ getVblankToFrameEndMs NOTIFY updated)
  Q_PROPERTY(double frameEndSpreadMs READ getFrameEndSpreadMs NOTIFY updated)
  Q_PROPERTY(double vblankToFrameGrabMs READ getVblankToFrameGrabMs NOTIFY updated)
  Q_PROPERTY(double frameGrabSpreadMs READ getFrameGrabSpreadMs NOTIFY updated)
  Q_PROPERTY(double presentsPerBlank READ getPresentsPerBlank NOTIFY updated)
  Q_PROPERTY(double framesPerBlank READ getFramesPerBlank NOTIFY updated)
  Q_PROPERTY(qint64 blanksWithoutPresent READ getBlanksWithoutPresent NOTIFY updated)
  Q_PROPERTY(qint64 blanksWithTwoPresents READ getBlanksWithTwoPresents NOTIFY updated)
  Q_PROPERTY(qint64 blanksWithoutFrame READ getBlanksWithoutFrame NOTIFY updated)
  Q_PROPERTY(qint64 blanksWithTwoFrames READ getBlanksWithTwoFrames NOTIFY updated)
  Q_PROPERTY(double spinMarginMs READ getSpinMarginMs NOTIFY updated)
  Q_PROPERTY(double wakeOvershootMeanMs READ getWakeOvershootMeanMs NOTIFY updated)
  Q_PROPERTY(double wakeOvershootPeakMs READ getWakeOvershootPeakMs NOTIFY updated)
  Q_PROPERTY(bool phaseLocked READ isPhaseLocked NOTIFY updated)
  Q_PROPERTY(double refreshPeriodMs READ getRefreshPeriodMs NOTIFY updated)
  Q_PROPERTY(qint64 slips READ getSlips NOTIFY updated)
  Q_PROPERTY(double slipDebtMs READ getSlipDebtMs NOTIFY updated)
  Q_PROPERTY(qint64 timeDrops READ getTimeDrops NOTIFY updated)
  Q_PROPERTY(qint64 framesRun READ getFramesRun NOTIFY updated)
  Q_PROPERTY(qint64 framesLost READ getFramesLost NOTIFY updated)
  Q_PROPERTY(qint64 framesNotShown READ getFramesNotShown NOTIFY updated)
  Q_PROPERTY(qint64 framesRepeated READ getFramesRepeated NOTIFY updated)
  Q_PROPERTY(qint64 framesNoPicture READ getFramesNoPicture NOTIFY updated)
  Q_PROPERTY(qint64 framesShown READ getFramesShown NOTIFY updated)
  Q_PROPERTY(qint64 repeatsNearSlip READ getRepeatsNearSlip NOTIFY updated)
  Q_PROPERTY(qint64 blanks READ getBlanks NOTIFY updated)
  Q_PROPERTY(qint64 presents READ getPresents NOTIFY updated)
  Q_PROPERTY(qint64 passes READ getPasses NOTIFY updated)

  Q_PROPERTY(QString audioDevice READ getAudioDevice NOTIFY updated)
  Q_PROPERTY(int bufferCapacityBytes READ getBufferCapacityBytes NOTIFY updated)
  Q_PROPERTY(double bufferSaturationPercent READ getBufferSaturationPercent NOTIFY updated)
  Q_PROPERTY(double bufferDeviationPercent READ getBufferDeviationPercent NOTIFY updated)
  Q_PROPERTY(double closeToUnderrunPercent READ getCloseToUnderrunPercent NOTIFY updated)
  Q_PROPERTY(double closeToBlockingPercent READ getCloseToBlockingPercent NOTIFY updated)
  Q_PROPERTY(double correctionPercent READ getCorrectionPercent NOTIFY updated)
  Q_PROPERTY(qint64 samplesDelivered READ getSamplesDelivered NOTIFY updated)

  Q_PROPERTY(qint64 lappedRecords READ getLappedRecords NOTIFY updated)
  Q_PROPERTY(qint64 droppedRecords READ getDroppedRecords NOTIFY updated)
  Q_PROPERTY(QVariantList kinds READ getKinds NOTIFY kindsChanged)

public:
  explicit QtPerformanceMonitorProxy(QObject *parent = nullptr);

  ~QtPerformanceMonitorProxy() override = default;

  /**
   * Shows or hides the panel, which is also what starts and stops recording
   */
  Q_INVOKABLE void toggle();

  /**
   * Freezes or resumes the readout and the charts; nothing is recorded while frozen
   */
  Q_INVOKABLE void togglePaused();

  /**
   * @return The source for the series registered under name, owned by this object
   */
  Q_INVOKABLE firelight::gui::SeriesSource *series(const QString &name);

  /**
   * @return The history the charts draw from
   */
  [[nodiscard]] const diagnostics::MonitorHistory &getHistory() const;

  [[nodiscard]] bool isVisible() const;
  void setVisible(bool visible);

  [[nodiscard]] bool isPaused() const;
  void setPaused(bool paused);

  [[nodiscard]] int getBaseWidth() const;
  [[nodiscard]] int getBaseHeight() const;
  [[nodiscard]] int getMaxWidth() const;
  [[nodiscard]] int getMaxHeight() const;
  [[nodiscard]] double getAspectRatio() const;
  [[nodiscard]] double getCoreFps() const;
  [[nodiscard]] double getCoreSampleRate() const;

  [[nodiscard]] QString getGraphicsApi() const;
  [[nodiscard]] int getViewportWidth() const;
  [[nodiscard]] int getViewportHeight() const;
  [[nodiscard]] int getRenderWidth() const;
  [[nodiscard]] int getRenderHeight() const;
  [[nodiscard]] double getDisplayHz() const;
  [[nodiscard]] double getTargetFps() const;
  [[nodiscard]] QString getPacingMode() const;
  [[nodiscard]] double getAudioRatio() const;

  [[nodiscard]] double getFrameRate() const;
  [[nodiscard]] bool isRateMatchingTarget() const;
  [[nodiscard]] double getFrameTimeMs() const;
  [[nodiscard]] double getFrameTimeDeviationPercent() const;
  [[nodiscard]] double getSubmitTimeMs() const;
  [[nodiscard]] double getSubmitDeviationPercent() const;
  [[nodiscard]] double getSpinMarginMs() const;
  [[nodiscard]] double getWakeOvershootMeanMs() const;
  [[nodiscard]] double getWakeOvershootPeakMs() const;
  [[nodiscard]] bool isPhaseLocked() const;
  [[nodiscard]] double getRefreshPeriodMs() const;
  [[nodiscard]] qint64 getSlips() const;

  [[nodiscard]] double getSlipDebtMs() const;

  [[nodiscard]] qint64 getTimeDrops() const;
  [[nodiscard]] qint64 getFramesRun() const;
  [[nodiscard]] qint64 getFramesLost() const;
  [[nodiscard]] qint64 getFramesNotShown() const;
  [[nodiscard]] qint64 getFramesRepeated() const;

  [[nodiscard]] qint64 getFramesNoPicture() const;

  [[nodiscard]] qint64 getFramesShown() const;

  [[nodiscard]] qint64 getRepeatsNearSlip() const;

  [[nodiscard]] qint64 getBlanks() const;

  [[nodiscard]] qint64 getPresents() const;

  [[nodiscard]] qint64 getPasses() const;

  [[nodiscard]] double getVblankToSubmitMs() const;

  [[nodiscard]] double getVblankToWaitEndMs() const;

  [[nodiscard]] double getVblankToFrameEndMs() const;

  [[nodiscard]] double getFrameEndSpreadMs() const;

  [[nodiscard]] double getVblankToFrameGrabMs() const;

  [[nodiscard]] double getFrameGrabSpreadMs() const;

  [[nodiscard]] double getPresentsPerBlank() const;

  [[nodiscard]] double getFramesPerBlank() const;
  [[nodiscard]] qint64 getBlanksWithoutPresent() const;
  [[nodiscard]] qint64 getBlanksWithTwoPresents() const;
  [[nodiscard]] qint64 getBlanksWithoutFrame() const;
  [[nodiscard]] qint64 getBlanksWithTwoFrames() const;

  [[nodiscard]] QString getAudioDevice() const;
  [[nodiscard]] int getBufferCapacityBytes() const;
  [[nodiscard]] double getBufferSaturationPercent() const;
  [[nodiscard]] double getBufferDeviationPercent() const;
  [[nodiscard]] double getCloseToUnderrunPercent() const;
  [[nodiscard]] double getCloseToBlockingPercent() const;
  [[nodiscard]] double getCorrectionPercent() const;
  [[nodiscard]] qint64 getSamplesDelivered() const;

  [[nodiscard]] qint64 getLappedRecords() const;
  [[nodiscard]] qint64 getDroppedRecords() const;
  [[nodiscard]] QVariantList getKinds() const;

signals:
  void visibleChanged();
  void pausedChanged();
  void updated();
  void kindsChanged();

private:
  /** How often the history is pulled and the figures re-derived, in ms */
  static constexpr int REFRESH_INTERVAL_MS = 100;

  /**
   * Pulls what is new, re-derives the figures, and tells the panel and every series source
   */
  void refresh();

  QTimer m_timer;
  /**
   * Starts or stops recording and the refresh timer to match visible and paused
   */
  void applyRunningState();

  bool m_visible = false;
  bool m_paused = false;
  diagnostics::MonitorHistory m_history;
  diagnostics::MonitorFigures m_figures;
  diagnostics::PerformanceSnapshot m_info;
  std::map<std::string, MonitorSeriesSource *> m_sources;
  size_t m_kindCount = 0;
};

} // namespace firelight::gui

// TODO: NEEDS REVIEW
#include "qt_performance_monitor_proxy.hpp"

#include <QVariantMap>
#include <cmath>

namespace firelight::gui {

//****************
// MonitorSeriesSource
//****************

MonitorSeriesSource::MonitorSeriesSource(const diagnostics::MonitorHistory &history, std::string name, QObject *parent)
    : SeriesSource(parent), m_history(history), m_name(std::move(name)) {}

void MonitorSeriesSource::copyPoints(const int64_t sinceNs, std::vector<SeriesPoint> &out) const {
  const auto kind = m_history.getKind(m_name);

  if (kind == 0) {
    return;
  }

  for (const auto &sample : m_history.getSamples()) {
    if (sample.kind == kind && sample.atNs >= sinceNs) {
      out.push_back({sample.atNs, sample.value});
    }
  }
}

int64_t MonitorSeriesSource::getNowNs() const { return m_history.getUpdatedNs(); }

//****************
// QtPerformanceMonitorProxy
//****************

QtPerformanceMonitorProxy::QtPerformanceMonitorProxy(QObject *parent)
    : QObject(parent), m_history(monitoring::Monitor::instance()) {
  m_timer.setInterval(REFRESH_INTERVAL_MS);
  connect(&m_timer, &QTimer::timeout, this, &QtPerformanceMonitorProxy::refresh);
}

void QtPerformanceMonitorProxy::toggle() { setVisible(!m_visible); }

SeriesSource *QtPerformanceMonitorProxy::series(const QString &name) {
  const auto key = name.toStdString();
  const auto found = m_sources.find(key);

  if (found != m_sources.end()) {
    return found->second;
  }

  auto *source = new MonitorSeriesSource(m_history, key, this);
  m_sources[key] = source;
  return source;
}

const diagnostics::MonitorHistory &QtPerformanceMonitorProxy::getHistory() const { return m_history; }

bool QtPerformanceMonitorProxy::isVisible() const { return m_visible; }

void QtPerformanceMonitorProxy::setVisible(const bool visible) {
  if (m_visible == visible) {
    return;
  }

  m_visible = visible;

  if (m_visible) {
    m_history.reset();
  }

  applyRunningState();
  emit visibleChanged();
}

void QtPerformanceMonitorProxy::togglePaused() { setPaused(!m_paused); }

bool QtPerformanceMonitorProxy::isPaused() const { return m_paused; }

void QtPerformanceMonitorProxy::setPaused(const bool paused) {
  if (m_paused == paused) {
    return;
  }

  m_paused = paused;
  applyRunningState();
  emit pausedChanged();
}

void QtPerformanceMonitorProxy::applyRunningState() {
  const auto running = m_visible && !m_paused;

  if (running) {
    monitoring::Monitor::instance().setRecording(true);
    refresh();
    m_timer.start();
  } else {
    m_timer.stop();
    monitoring::Monitor::instance().setRecording(false);
  }
}

void QtPerformanceMonitorProxy::refresh() {
  m_history.update();
  m_figures = m_history.getFigures();
  m_info = diagnostics::PerformanceStats::instance().snapshot();
  emit updated();

  for (const auto &[name, source] : m_sources) {
    emit source->changed();
  }

  if (const auto kindCount = monitoring::Monitor::instance().getKinds().size(); kindCount != m_kindCount) {
    m_kindCount = kindCount;
    emit kindsChanged();
  }
}

int QtPerformanceMonitorProxy::getBaseWidth() const { return m_info.baseWidth; }

int QtPerformanceMonitorProxy::getBaseHeight() const { return m_info.baseHeight; }

int QtPerformanceMonitorProxy::getMaxWidth() const { return m_info.maxWidth; }

int QtPerformanceMonitorProxy::getMaxHeight() const { return m_info.maxHeight; }

double QtPerformanceMonitorProxy::getAspectRatio() const { return m_info.aspectRatio; }

double QtPerformanceMonitorProxy::getCoreFps() const { return m_info.coreFps; }

double QtPerformanceMonitorProxy::getCoreSampleRate() const { return m_info.coreSampleRate; }

QString QtPerformanceMonitorProxy::getGraphicsApi() const { return QString::fromStdString(m_info.graphicsApi); }

int QtPerformanceMonitorProxy::getViewportWidth() const { return m_info.viewportWidth; }

int QtPerformanceMonitorProxy::getViewportHeight() const { return m_info.viewportHeight; }

int QtPerformanceMonitorProxy::getRenderWidth() const { return m_info.renderWidth; }

int QtPerformanceMonitorProxy::getRenderHeight() const { return m_info.renderHeight; }

double QtPerformanceMonitorProxy::getDisplayHz() const { return m_info.displayHz; }

double QtPerformanceMonitorProxy::getTargetFps() const { return m_info.targetFps; }

QString QtPerformanceMonitorProxy::getPacingMode() const { return QString::fromStdString(m_info.pacingMode); }

double QtPerformanceMonitorProxy::getAudioRatio() const { return m_info.audioRatio; }

double QtPerformanceMonitorProxy::getFrameRate() const { return m_figures.frameRate; }

// TODO
// A tenth of a percent is inside what a mode may round to and outside what a mode silently running
// at the wrong rate produces
bool QtPerformanceMonitorProxy::isRateMatchingTarget() const {
  if (m_info.targetFps <= 0.0 || m_figures.frameRate <= 0.0) {
    return true;
  }

  return std::abs(m_figures.frameRate - m_info.targetFps) / m_info.targetFps < 0.001;
}

double QtPerformanceMonitorProxy::getFrameTimeMs() const { return m_figures.frameTimeMs; }

double QtPerformanceMonitorProxy::getFrameTimeDeviationPercent() const { return m_figures.frameTimeDeviationPercent; }

double QtPerformanceMonitorProxy::getSubmitTimeMs() const { return m_figures.submitTimeMs; }

double QtPerformanceMonitorProxy::getSubmitDeviationPercent() const { return m_figures.submitDeviationPercent; }

double QtPerformanceMonitorProxy::getSpinMarginMs() const { return m_figures.spinMarginMs; }

double QtPerformanceMonitorProxy::getWakeOvershootMeanMs() const { return m_figures.wakeOvershootMeanMs; }

double QtPerformanceMonitorProxy::getWakeOvershootPeakMs() const { return m_figures.wakeOvershootPeakMs; }

bool QtPerformanceMonitorProxy::isPhaseLocked() const { return m_figures.phaseLocked; }

double QtPerformanceMonitorProxy::getRefreshPeriodMs() const { return m_figures.refreshPeriodMs; }

qint64 QtPerformanceMonitorProxy::getSlips() const { return m_figures.slips; }

double QtPerformanceMonitorProxy::getSlipDebtMs() const { return m_figures.slipDebtMs; }

qint64 QtPerformanceMonitorProxy::getTimeDrops() const { return m_figures.timeDrops; }

qint64 QtPerformanceMonitorProxy::getFramesRun() const { return m_figures.framesRun; }

qint64 QtPerformanceMonitorProxy::getFramesLost() const { return m_figures.framesLost; }

qint64 QtPerformanceMonitorProxy::getFramesNotShown() const { return m_figures.framesNotShown; }

qint64 QtPerformanceMonitorProxy::getFramesRepeated() const { return m_figures.framesRepeated; }

qint64 QtPerformanceMonitorProxy::getFramesNoPicture() const { return m_figures.framesNoPicture; }

qint64 QtPerformanceMonitorProxy::getFramesShown() const { return m_figures.framesShown; }

qint64 QtPerformanceMonitorProxy::getRepeatsNearSlip() const { return m_figures.repeatsNearSlip; }

qint64 QtPerformanceMonitorProxy::getBlanks() const { return m_figures.blanks; }

qint64 QtPerformanceMonitorProxy::getPresents() const { return m_figures.presents; }

qint64 QtPerformanceMonitorProxy::getPasses() const { return m_figures.passes; }

double QtPerformanceMonitorProxy::getVblankToSubmitMs() const { return m_figures.vblankToSubmitMs; }

double QtPerformanceMonitorProxy::getVblankToWaitEndMs() const { return m_figures.vblankToWaitEndMs; }

double QtPerformanceMonitorProxy::getVblankToFrameEndMs() const { return m_figures.vblankToFrameEndMs; }

double QtPerformanceMonitorProxy::getFrameEndSpreadMs() const { return m_figures.frameEndSpreadMs; }

double QtPerformanceMonitorProxy::getVblankToFrameGrabMs() const { return m_figures.vblankToFrameGrabMs; }

double QtPerformanceMonitorProxy::getFrameGrabSpreadMs() const { return m_figures.frameGrabSpreadMs; }

double QtPerformanceMonitorProxy::getPresentsPerBlank() const { return m_figures.presentsPerBlank; }

double QtPerformanceMonitorProxy::getFramesPerBlank() const { return m_figures.framesPerBlank; }

qint64 QtPerformanceMonitorProxy::getBlanksWithoutPresent() const { return m_figures.blanksWithoutPresent; }

qint64 QtPerformanceMonitorProxy::getBlanksWithTwoPresents() const { return m_figures.blanksWithTwoPresents; }

qint64 QtPerformanceMonitorProxy::getBlanksWithoutFrame() const { return m_figures.blanksWithoutFrame; }

qint64 QtPerformanceMonitorProxy::getBlanksWithTwoFrames() const { return m_figures.blanksWithTwoFrames; }

QString QtPerformanceMonitorProxy::getAudioDevice() const { return QString::fromStdString(m_info.audioDevice); }

int QtPerformanceMonitorProxy::getBufferCapacityBytes() const { return m_info.bufferCapacityBytes; }

double QtPerformanceMonitorProxy::getBufferSaturationPercent() const { return m_figures.bufferSaturationPercent; }

double QtPerformanceMonitorProxy::getBufferDeviationPercent() const { return m_figures.bufferDeviationPercent; }

double QtPerformanceMonitorProxy::getCloseToUnderrunPercent() const { return m_figures.closeToUnderrunPercent; }

double QtPerformanceMonitorProxy::getCloseToBlockingPercent() const { return m_figures.closeToBlockingPercent; }

double QtPerformanceMonitorProxy::getCorrectionPercent() const { return m_figures.correctionPercent; }

qint64 QtPerformanceMonitorProxy::getSamplesDelivered() const { return m_figures.samplesDelivered; }

qint64 QtPerformanceMonitorProxy::getLappedRecords() const { return static_cast<qint64>(m_figures.lappedRecords); }

qint64 QtPerformanceMonitorProxy::getDroppedRecords() const { return static_cast<qint64>(m_figures.droppedRecords); }

QVariantList QtPerformanceMonitorProxy::getKinds() const {
  QVariantList kinds;

  for (const auto &kind : monitoring::Monitor::instance().getKinds()) {
    QVariantMap entry;
    entry["id"] = kind.id;
    entry["name"] = QString::fromStdString(kind.name);
    entry["summary"] = QString::fromStdString(kind.summary);
    entry["type"] = kind.type == monitoring::KindType::Span     ? QStringLiteral("span")
                    : kind.type == monitoring::KindType::Marker ? QStringLiteral("marker")
                                                                : QStringLiteral("series");
    kinds.push_back(entry);
  }

  return kinds;
}

} // namespace firelight::gui

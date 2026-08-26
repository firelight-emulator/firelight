// TODO: NEEDS REVIEW
#include "qt_performance_stats_proxy.hpp"

namespace firelight::gui {

namespace {
/**
 * Formats a number the way the overlay shows it, or a dash when nothing has been measured
 */
QString figure(const double value, const int decimals, const QString &suffix = {}) {
  return QString::number(value, 'f', decimals) + suffix;
}

/**
 * Formats a rate or a size, which reads as absent rather than as zero when nothing has set it yet
 */
QString figureOrDash(const double value, const int decimals, const QString &suffix = {}) {
  if (value <= 0.0) {
    return QStringLiteral("—");
  }

  return figure(value, decimals, suffix);
}
} // namespace

QtPerformanceStatsProxy::QtPerformanceStatsProxy(QObject *parent) : QObject(parent) {
  m_timer.setInterval(REFRESH_INTERVAL_MS);
  connect(&m_timer, &QTimer::timeout, this, &QtPerformanceStatsProxy::refresh);
}

void QtPerformanceStatsProxy::toggle() { setVisible(!m_visible); }

bool QtPerformanceStatsProxy::isVisible() const { return m_visible; }

void QtPerformanceStatsProxy::setVisible(const bool visible) {
  if (m_visible == visible) {
    return;
  }

  m_visible = visible;

  if (m_visible) {
    refresh();
    m_timer.start();
  } else {
    m_timer.stop();
  }

  emit visibleChanged();
}

void QtPerformanceStatsProxy::refresh() {
  m_snapshot = diagnostics::PerformanceStats::instance().snapshot();
  emit updated();
}

QString QtPerformanceStatsProxy::getCoreBase() const {
  return QStringLiteral("%1 x %2").arg(m_snapshot.baseWidth).arg(m_snapshot.baseHeight);
}

QString QtPerformanceStatsProxy::getCoreMax() const {
  return QStringLiteral("%1 x %2").arg(m_snapshot.maxWidth).arg(m_snapshot.maxHeight);
}

QString QtPerformanceStatsProxy::getAspectRatio() const { return figureOrDash(m_snapshot.aspectRatio, 3); }

QString QtPerformanceStatsProxy::getCoreFps() const { return figureOrDash(m_snapshot.coreFps, 3); }

QString QtPerformanceStatsProxy::getSampleRate() const { return figureOrDash(m_snapshot.coreSampleRate, 2); }

QString QtPerformanceStatsProxy::getGraphicsApi() const {
  return m_snapshot.graphicsApi.empty() ? QStringLiteral("—") : QString::fromStdString(m_snapshot.graphicsApi);
}

QString QtPerformanceStatsProxy::getViewport() const {
  return QStringLiteral("%1 x %2").arg(m_snapshot.viewportWidth).arg(m_snapshot.viewportHeight);
}

QString QtPerformanceStatsProxy::getRenderSize() const {
  return QStringLiteral("%1 x %2").arg(m_snapshot.renderWidth).arg(m_snapshot.renderHeight);
}

QString QtPerformanceStatsProxy::getRefreshRate() const {
  return figureOrDash(m_snapshot.displayHz, 3, QStringLiteral(" Hz"));
}

QString QtPerformanceStatsProxy::getFrameRate() const {
  return figureOrDash(m_snapshot.frameRate, 3, QStringLiteral(" fps"));
}

QString QtPerformanceStatsProxy::getFrameTime() const {
  return figureOrDash(m_snapshot.frameTimeMs, 2, QStringLiteral(" ms"));
}

QString QtPerformanceStatsProxy::getFrameTimeDeviation() const {
  return figure(m_snapshot.frameTimeDeviationPercent, 2, QStringLiteral(" %"));
}

QString QtPerformanceStatsProxy::getSubmitTime() const {
  return figureOrDash(m_snapshot.submitTimeMs, 2, QStringLiteral(" ms"));
}

QString QtPerformanceStatsProxy::getSubmitDeviation() const {
  return figure(m_snapshot.submitDeviationPercent, 2, QStringLiteral(" %"));
}

QString QtPerformanceStatsProxy::getSpinMargin() const {
  return figure(m_snapshot.spinMarginMs, 2, QStringLiteral(" ms"));
}

QString QtPerformanceStatsProxy::getWakeOvershoot() const {
  return QStringLiteral("%1 / %2 ms")
      .arg(m_snapshot.wakeOvershootMeanMs, 0, 'f', 2)
      .arg(m_snapshot.wakeOvershootPeakMs, 0, 'f', 2);
}

QString QtPerformanceStatsProxy::getFramesRun() const { return QString::number(m_snapshot.framesRun); }

QString QtPerformanceStatsProxy::getFramesLost() const { return QString::number(m_snapshot.framesLost); }

QString QtPerformanceStatsProxy::getPacingMode() const {
  return m_snapshot.pacingMode.empty() ? QStringLiteral("—") : QString::fromStdString(m_snapshot.pacingMode);
}

QString QtPerformanceStatsProxy::getAudioRatio() const {
  return QStringLiteral("x") + QString::number(m_snapshot.audioRatio, 'f', 4);
}

QString QtPerformanceStatsProxy::getAudioDevice() const {
  return m_snapshot.audioDevice.empty() ? QStringLiteral("—") : QString::fromStdString(m_snapshot.audioDevice);
}

QString QtPerformanceStatsProxy::getBufferCapacity() const {
  if (m_snapshot.bufferCapacityBytes <= 0 || m_snapshot.coreSampleRate <= 0.0) {
    return QStringLiteral("—");
  }

  return QStringLiteral("%1 B").arg(m_snapshot.bufferCapacityBytes);
}

QString QtPerformanceStatsProxy::getSaturation() const {
  return figure(m_snapshot.bufferSaturationPercent, 2, QStringLiteral(" %"));
}

QString QtPerformanceStatsProxy::getSaturationDeviation() const {
  return figure(m_snapshot.bufferDeviationPercent, 2, QStringLiteral(" %"));
}

QString QtPerformanceStatsProxy::getCloseToUnderrun() const {
  return QString::number(m_snapshot.closeToUnderrunPercent, 'f', 2) + QStringLiteral(" %");
}

QString QtPerformanceStatsProxy::getCloseToBlocking() const {
  return QString::number(m_snapshot.closeToBlockingPercent, 'f', 2) + QStringLiteral(" %");
}

QString QtPerformanceStatsProxy::getCorrection() const {
  return QString::asprintf("%+.4f %%", m_snapshot.correctionPercent);
}

QString QtPerformanceStatsProxy::getSamples() const { return QString::number(m_snapshot.samplesDelivered); }

} // namespace firelight::gui

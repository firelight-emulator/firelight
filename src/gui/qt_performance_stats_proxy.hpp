// TODO: NEEDS REVIEW
#pragma once

#include "diagnostics/performance_stats.hpp"

#include <QObject>
#include <QString>
#include <QTimer>

namespace firelight::gui {

/**
 * Exposes what the emulator is doing to the overlay that shows it.
 *
 * Polled rather than pushed: the numbers are produced on the render thread once per frame, and a
 * signal per frame would cost more than the overlay is worth. Nothing is read while the overlay is
 * closed
 */
class QtPerformanceStatsProxy final : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged)

  Q_PROPERTY(QString coreBase READ getCoreBase NOTIFY updated)
  Q_PROPERTY(QString coreMax READ getCoreMax NOTIFY updated)
  Q_PROPERTY(QString aspectRatio READ getAspectRatio NOTIFY updated)
  Q_PROPERTY(QString coreFps READ getCoreFps NOTIFY updated)
  Q_PROPERTY(QString sampleRate READ getSampleRate NOTIFY updated)

  Q_PROPERTY(QString graphicsApi READ getGraphicsApi NOTIFY updated)
  Q_PROPERTY(QString viewport READ getViewport NOTIFY updated)
  Q_PROPERTY(QString renderSize READ getRenderSize NOTIFY updated)
  Q_PROPERTY(QString refreshRate READ getRefreshRate NOTIFY updated)
  Q_PROPERTY(QString frameRate READ getFrameRate NOTIFY updated)
  Q_PROPERTY(QString frameTime READ getFrameTime NOTIFY updated)
  Q_PROPERTY(QString frameTimeDeviation READ getFrameTimeDeviation NOTIFY updated)
  Q_PROPERTY(QString presentTime READ getPresentTime NOTIFY updated)
  Q_PROPERTY(QString presentDeviation READ getPresentDeviation NOTIFY updated)
  Q_PROPERTY(QString spinMargin READ getSpinMargin NOTIFY updated)
  Q_PROPERTY(QString wakeOvershoot READ getWakeOvershoot NOTIFY updated)
  Q_PROPERTY(QString framesRun READ getFramesRun NOTIFY updated)
  Q_PROPERTY(QString framesLost READ getFramesLost NOTIFY updated)
  Q_PROPERTY(QString pacingMode READ getPacingMode NOTIFY updated)
  Q_PROPERTY(QString audioRatio READ getAudioRatio NOTIFY updated)

  Q_PROPERTY(QString audioDevice READ getAudioDevice NOTIFY updated)
  Q_PROPERTY(QString bufferCapacity READ getBufferCapacity NOTIFY updated)
  Q_PROPERTY(QString saturation READ getSaturation NOTIFY updated)
  Q_PROPERTY(QString saturationDeviation READ getSaturationDeviation NOTIFY updated)
  Q_PROPERTY(QString closeToUnderrun READ getCloseToUnderrun NOTIFY updated)
  Q_PROPERTY(QString closeToBlocking READ getCloseToBlocking NOTIFY updated)
  Q_PROPERTY(QString correction READ getCorrection NOTIFY updated)
  Q_PROPERTY(QString samples READ getSamples NOTIFY updated)

public:
  explicit QtPerformanceStatsProxy(QObject *parent = nullptr);
  ~QtPerformanceStatsProxy() override = default;

  /**
   * Toggles the overlay, which is also what starts and stops reading for it
   */
  Q_INVOKABLE void toggle();

  [[nodiscard]] bool isVisible() const;
  void setVisible(bool visible);

  [[nodiscard]] QString getCoreBase() const;
  [[nodiscard]] QString getCoreMax() const;
  [[nodiscard]] QString getAspectRatio() const;
  [[nodiscard]] QString getCoreFps() const;
  [[nodiscard]] QString getSampleRate() const;

  [[nodiscard]] QString getGraphicsApi() const;
  [[nodiscard]] QString getViewport() const;
  [[nodiscard]] QString getRenderSize() const;
  [[nodiscard]] QString getRefreshRate() const;
  [[nodiscard]] QString getFrameRate() const;
  [[nodiscard]] QString getFrameTime() const;
  [[nodiscard]] QString getFrameTimeDeviation() const;
  [[nodiscard]] QString getPresentTime() const;
  [[nodiscard]] QString getPresentDeviation() const;
  [[nodiscard]] QString getSpinMargin() const;
  [[nodiscard]] QString getWakeOvershoot() const;
  [[nodiscard]] QString getFramesRun() const;
  [[nodiscard]] QString getFramesLost() const;
  [[nodiscard]] QString getPacingMode() const;
  [[nodiscard]] QString getAudioRatio() const;

  [[nodiscard]] QString getAudioDevice() const;
  [[nodiscard]] QString getBufferCapacity() const;
  [[nodiscard]] QString getSaturation() const;
  [[nodiscard]] QString getSaturationDeviation() const;
  [[nodiscard]] QString getCloseToUnderrun() const;
  [[nodiscard]] QString getCloseToBlocking() const;
  [[nodiscard]] QString getCorrection() const;
  [[nodiscard]] QString getSamples() const;

signals:
  void visibleChanged();
  void updated();

private:
  // TODO
  // Fast enough that a hitch is still visible in the numbers, slow enough that the figures hold
  // still long enough to be read off the screen against another emulator's
  static constexpr int REFRESH_INTERVAL_MS = 250;

  /**
   * Takes a fresh copy and tells the overlay to re-read everything
   */
  void refresh();

  QTimer m_timer;
  bool m_visible = false;
  diagnostics::PerformanceSnapshot m_snapshot;
};

} // namespace firelight::gui

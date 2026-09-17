// TODO: NEEDS REVIEW
#pragma once

#include "chart_quads.hpp"
#include "qt_performance_monitor_proxy.hpp"

#include <QColor>
#include <QPointer>
#include <QQuickItem>
#include <QVariantList>
#include <QVariantMap>
#include <mutex>
#include <vector>

namespace firelight::gui {

/**
 * One bar per interval between two boundary markers, with every span that overlapped the interval
 * drawn inside it where it happened, one lane per nesting depth. Newest bar on the right
 */
class SpanChartItem : public QQuickItem {
  Q_OBJECT
  Q_PROPERTY(QObject *source READ getSource WRITE setSource NOTIFY sourceChanged)
  Q_PROPERTY(QString boundaryKind READ getBoundaryKind WRITE setBoundaryKind NOTIFY boundaryKindChanged)
  Q_PROPERTY(double windowSeconds READ getWindowSeconds WRITE setWindowSeconds NOTIFY layoutChanged)
  Q_PROPERTY(double yMaxMs READ getYMaxMs WRITE setYMaxMs NOTIFY rangeChanged)
  Q_PROPERTY(bool autoScale READ isAutoScale WRITE setAutoScale NOTIFY rangeChanged)
  Q_PROPERTY(double effectiveYMaxMs READ getEffectiveYMaxMs NOTIFY effectiveRangeChanged)
  Q_PROPERTY(double barWidth READ getBarWidth WRITE setBarWidth NOTIFY layoutChanged)
  Q_PROPERTY(double barGap READ getBarGap WRITE setBarGap NOTIFY layoutChanged)
  Q_PROPERTY(QVariantList seriesColors READ getSeriesColors WRITE setSeriesColors NOTIFY styleChanged)
  Q_PROPERTY(QColor barColor READ getBarColor WRITE setBarColor NOTIFY styleChanged)
  Q_PROPERTY(QVariantList guideLines READ getGuideLines WRITE setGuideLines NOTIFY decorationsChanged)
  Q_PROPERTY(QVariantList bands READ getBands WRITE setBands NOTIFY decorationsChanged)
  Q_PROPERTY(QVariantMap hoveredSpan READ getHoveredSpan NOTIFY hoveredSpanChanged)
  Q_PROPERTY(int barCount READ getBarCount NOTIFY barCountChanged)

public:
  explicit SpanChartItem(QQuickItem *parent = nullptr);

  [[nodiscard]] QObject *getSource() const;
  void setSource(QObject *source);

  [[nodiscard]] QString getBoundaryKind() const;
  void setBoundaryKind(const QString &kind);

  [[nodiscard]] double getWindowSeconds() const;
  void setWindowSeconds(double seconds);

  [[nodiscard]] double getYMaxMs() const;
  void setYMaxMs(double value);

  [[nodiscard]] bool isAutoScale() const;
  void setAutoScale(bool autoScale);

  [[nodiscard]] double getEffectiveYMaxMs() const;

  [[nodiscard]] double getBarWidth() const;
  void setBarWidth(double width);

  [[nodiscard]] double getBarGap() const;
  void setBarGap(double gap);

  [[nodiscard]] QVariantList getSeriesColors() const;
  void setSeriesColors(const QVariantList &colors);

  [[nodiscard]] QColor getBarColor() const;
  void setBarColor(const QColor &color);

  [[nodiscard]] QVariantList getGuideLines() const;
  void setGuideLines(const QVariantList &guideLines);

  [[nodiscard]] QVariantList getBands() const;
  void setBands(const QVariantList &bands);

  [[nodiscard]] QVariantMap getHoveredSpan() const;

  [[nodiscard]] int getBarCount() const;

signals:
  void sourceChanged();
  void boundaryKindChanged();
  void layoutChanged();
  void rangeChanged();
  void effectiveRangeChanged();
  void styleChanged();
  void decorationsChanged();
  void hoveredSpanChanged();
  void barCountChanged();

protected:
  QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data) override;

  void hoverMoveEvent(QHoverEvent *event) override;

  void hoverLeaveEvent(QHoverEvent *event) override;

  void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

private:
  /** One drawn segment, kept so a hover can name it */
  struct Segment {
    QRectF rect;
    monitoring::KindId kind = 0;
    int bar = 0;
    double offsetMs = 0.0;
    double durationMs = 0.0;
    int depth = 0;
  };

  /**
   * Slices the history into bars and turns them into quads, then schedules a repaint
   */
  void rebuild();

  /**
   * @return The colour for a kind, from seriesColors by registration order
   */
  [[nodiscard]] QColor colorForKind(monitoring::KindId kind) const;

  QPointer<QtPerformanceMonitorProxy> m_proxy;
  QString m_boundaryKind = QStringLiteral("present");
  double m_windowSeconds = 5.0;
  double m_yMaxMs = 33.4;
  bool m_autoScale = false;
  double m_effectiveYMaxMs = 33.4;
  double m_barWidth = 6.0;
  double m_barGap = 1.0;
  QVariantList m_seriesColors;
  QColor m_barColor = QColor(255, 255, 255, 40);
  QVariantList m_guideLines;
  QVariantList m_bands;
  QVariantMap m_hoveredSpan;
  int m_barCount = 0;
  std::vector<Segment> m_segments;

  std::mutex m_quadMutex;
  QVector<Quad> m_quads;
  bool m_quadsDirty = false;
};

} // namespace firelight::gui

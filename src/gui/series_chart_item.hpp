// TODO: NEEDS REVIEW
#pragma once

#include "chart_quads.hpp"
#include "series_source.hpp"

#include <QColor>
#include <QPointer>
#include <QQuickItem>
#include <QVariantList>
#include <QVariantMap>
#include <mutex>
#include <vector>

namespace firelight::gui {

/**
 * A time-series line chart with an area fill, guide lines and bands. Draws whatever its source
 * gives it, or a static list of points; knows nothing about where the numbers come from
 */
class SeriesChartItem : public QQuickItem {
  Q_OBJECT
  Q_PROPERTY(firelight::gui::SeriesSource *source READ getSource WRITE setSource NOTIFY sourceChanged)
  Q_PROPERTY(QVariantList points READ getPoints WRITE setPoints NOTIFY pointsChanged)
  Q_PROPERTY(double windowSeconds READ getWindowSeconds WRITE setWindowSeconds NOTIFY windowSecondsChanged)
  Q_PROPERTY(double yMin READ getYMin WRITE setYMin NOTIFY rangeChanged)
  Q_PROPERTY(double yMax READ getYMax WRITE setYMax NOTIFY rangeChanged)
  Q_PROPERTY(bool autoScale READ isAutoScale WRITE setAutoScale NOTIFY rangeChanged)
  Q_PROPERTY(double effectiveYMin READ getEffectiveYMin NOTIFY effectiveRangeChanged)
  Q_PROPERTY(double effectiveYMax READ getEffectiveYMax NOTIFY effectiveRangeChanged)
  Q_PROPERTY(QColor lineColor READ getLineColor WRITE setLineColor NOTIFY styleChanged)
  Q_PROPERTY(QColor fillColor READ getFillColor WRITE setFillColor NOTIFY styleChanged)
  Q_PROPERTY(double lineWidth READ getLineWidth WRITE setLineWidth NOTIFY styleChanged)
  Q_PROPERTY(QVariantList guideLines READ getGuideLines WRITE setGuideLines NOTIFY decorationsChanged)
  Q_PROPERTY(QVariantList bands READ getBands WRITE setBands NOTIFY decorationsChanged)
  Q_PROPERTY(QVariantMap hoveredPoint READ getHoveredPoint NOTIFY hoveredPointChanged)

public:
  explicit SeriesChartItem(QQuickItem *parent = nullptr);

  [[nodiscard]] SeriesSource *getSource() const;
  void setSource(SeriesSource *source);

  [[nodiscard]] QVariantList getPoints() const;
  void setPoints(const QVariantList &points);

  [[nodiscard]] double getWindowSeconds() const;
  void setWindowSeconds(double seconds);

  [[nodiscard]] double getYMin() const;
  void setYMin(double value);

  [[nodiscard]] double getYMax() const;
  void setYMax(double value);

  [[nodiscard]] bool isAutoScale() const;
  void setAutoScale(bool autoScale);

  [[nodiscard]] double getEffectiveYMin() const;
  [[nodiscard]] double getEffectiveYMax() const;

  [[nodiscard]] QColor getLineColor() const;
  void setLineColor(const QColor &color);

  [[nodiscard]] QColor getFillColor() const;
  void setFillColor(const QColor &color);

  [[nodiscard]] double getLineWidth() const;
  void setLineWidth(double width);

  [[nodiscard]] QVariantList getGuideLines() const;
  void setGuideLines(const QVariantList &guideLines);

  [[nodiscard]] QVariantList getBands() const;
  void setBands(const QVariantList &bands);

  [[nodiscard]] QVariantMap getHoveredPoint() const;

signals:
  void sourceChanged();
  void pointsChanged();
  void windowSecondsChanged();
  void rangeChanged();
  void effectiveRangeChanged();
  void styleChanged();
  void decorationsChanged();
  void hoveredPointChanged();

protected:
  QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data) override;

  void hoverMoveEvent(QHoverEvent *event) override;

  void hoverLeaveEvent(QHoverEvent *event) override;

  void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

private:
  /**
   * Takes the current points from the source or the static list, in chart time
   */
  void gatherPoints();

  /**
   * Turns the points and decorations into quads for the render thread, then schedules a repaint
   */
  void rebuild();

  /**
   * Maps a value to a y pixel inside the item
   */
  [[nodiscard]] qreal valueToY(double value) const;

  /**
   * Maps a time to an x pixel inside the item
   */
  [[nodiscard]] qreal timeToX(int64_t atNs) const;

  QPointer<SeriesSource> m_source;
  QVariantList m_staticPoints;
  std::vector<SeriesPoint> m_points;
  int64_t m_windowEndNs = 0;
  double m_windowSeconds = 10.0;
  double m_yMin = 0.0;
  double m_yMax = 1.0;
  bool m_autoScale = false;
  double m_effectiveYMin = 0.0;
  double m_effectiveYMax = 1.0;
  QColor m_lineColor = QColor(255, 255, 255);
  QColor m_fillColor = QColor(255, 255, 255, 48);
  double m_lineWidth = 1.5;
  QVariantList m_guideLines;
  QVariantList m_bands;
  QVariantMap m_hoveredPoint;

  std::mutex m_quadMutex;
  QVector<Quad> m_quads;
  bool m_quadsDirty = false;
};

} // namespace firelight::gui

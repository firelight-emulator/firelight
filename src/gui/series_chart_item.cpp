// TODO: NEEDS REVIEW
#include "series_chart_item.hpp"

#include <QHoverEvent>
#include <algorithm>
#include <cmath>

namespace firelight::gui {

namespace {
constexpr double NS_PER_S = 1e9;
constexpr qreal GUIDE_THICKNESS = 1.0;
} // namespace

SeriesChartItem::SeriesChartItem(QQuickItem *parent) : QQuickItem(parent) {
  setFlag(ItemHasContents);
  setAcceptHoverEvents(true);
}

SeriesSource *SeriesChartItem::getSource() const { return m_source; }

void SeriesChartItem::setSource(SeriesSource *source) {
  if (m_source == source) {
    return;
  }

  if (m_source) {
    disconnect(m_source, nullptr, this, nullptr);
  }

  m_source = source;

  if (m_source) {
    connect(m_source, &SeriesSource::changed, this, &SeriesChartItem::rebuild);
  }

  emit sourceChanged();
  rebuild();
}

QVariantList SeriesChartItem::getPoints() const { return m_staticPoints; }

void SeriesChartItem::setPoints(const QVariantList &points) {
  m_staticPoints = points;
  emit pointsChanged();
  rebuild();
}

double SeriesChartItem::getWindowSeconds() const { return m_windowSeconds; }

void SeriesChartItem::setWindowSeconds(const double seconds) {
  if (m_windowSeconds == seconds) {
    return;
  }

  m_windowSeconds = seconds;
  emit windowSecondsChanged();
  rebuild();
}

double SeriesChartItem::getYMin() const { return m_yMin; }

void SeriesChartItem::setYMin(const double value) {
  m_yMin = value;
  emit rangeChanged();
  rebuild();
}

double SeriesChartItem::getYMax() const { return m_yMax; }

void SeriesChartItem::setYMax(const double value) {
  m_yMax = value;
  emit rangeChanged();
  rebuild();
}

bool SeriesChartItem::isAutoScale() const { return m_autoScale; }

void SeriesChartItem::setAutoScale(const bool autoScale) {
  m_autoScale = autoScale;
  emit rangeChanged();
  rebuild();
}

double SeriesChartItem::getEffectiveYMin() const { return m_effectiveYMin; }

double SeriesChartItem::getEffectiveYMax() const { return m_effectiveYMax; }

QColor SeriesChartItem::getLineColor() const { return m_lineColor; }

void SeriesChartItem::setLineColor(const QColor &color) {
  m_lineColor = color;
  emit styleChanged();
  rebuild();
}

QColor SeriesChartItem::getFillColor() const { return m_fillColor; }

void SeriesChartItem::setFillColor(const QColor &color) {
  m_fillColor = color;
  emit styleChanged();
  rebuild();
}

double SeriesChartItem::getLineWidth() const { return m_lineWidth; }

void SeriesChartItem::setLineWidth(const double width) {
  m_lineWidth = width;
  emit styleChanged();
  rebuild();
}

QVariantList SeriesChartItem::getGuideLines() const { return m_guideLines; }

void SeriesChartItem::setGuideLines(const QVariantList &guideLines) {
  m_guideLines = guideLines;
  emit decorationsChanged();
  rebuild();
}

QVariantList SeriesChartItem::getBands() const { return m_bands; }

void SeriesChartItem::setBands(const QVariantList &bands) {
  m_bands = bands;
  emit decorationsChanged();
  rebuild();
}

QVariantMap SeriesChartItem::getHoveredPoint() const { return m_hoveredPoint; }

QSGNode *SeriesChartItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) {
  auto *node = static_cast<QSGGeometryNode *>(oldNode);
  std::lock_guard lock(m_quadMutex);

  if (!node || m_quadsDirty) {
    node = updateQuadNode(node, m_quads);
    m_quadsDirty = false;
  }

  return node;
}

void SeriesChartItem::hoverMoveEvent(QHoverEvent *event) {
  if (m_points.empty() || width() <= 0.0) {
    return;
  }

  const auto x = event->position().x();
  auto best = m_points.front();
  auto bestDistance = std::abs(timeToX(best.atNs) - x);

  for (const auto &point : m_points) {
    const auto distance = std::abs(timeToX(point.atNs) - x);

    if (distance < bestDistance) {
      best = point;
      bestDistance = distance;
    }
  }

  QVariantMap hovered;
  hovered["value"] = best.value;
  hovered["secondsAgo"] = static_cast<double>(m_windowEndNs - best.atNs) / NS_PER_S;
  hovered["x"] = timeToX(best.atNs);
  hovered["y"] = valueToY(best.value);
  m_hoveredPoint = hovered;
  emit hoveredPointChanged();
}

void SeriesChartItem::hoverLeaveEvent(QHoverEvent *) {
  m_hoveredPoint.clear();
  emit hoveredPointChanged();
}

void SeriesChartItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) {
  QQuickItem::geometryChange(newGeometry, oldGeometry);
  rebuild();
}

void SeriesChartItem::gatherPoints() {
  m_points.clear();

  if (m_source) {
    m_windowEndNs = m_source->getNowNs();
    const auto sinceNs = m_windowEndNs - static_cast<int64_t>(m_windowSeconds * NS_PER_S);
    m_source->copyPoints(sinceNs, m_points);
    return;
  }

  // TODO
  // Static points carry their own x in seconds; the window ends at the last one
  for (const auto &entry : m_staticPoints) {
    const auto map = entry.toMap();
    m_points.push_back({static_cast<int64_t>(map.value("x").toDouble() * NS_PER_S), map.value("y").toDouble()});
  }

  std::sort(m_points.begin(), m_points.end(), [](const auto &a, const auto &b) { return a.atNs < b.atNs; });
  m_windowEndNs = m_points.empty() ? 0 : m_points.back().atNs;
}

void SeriesChartItem::rebuild() {
  gatherPoints();

  auto yMin = m_yMin;
  auto yMax = m_yMax;

  if (m_autoScale && !m_points.empty()) {
    yMin = m_points.front().value;
    yMax = yMin;

    for (const auto &point : m_points) {
      yMin = std::min(yMin, point.value);
      yMax = std::max(yMax, point.value);
    }

    if (yMax - yMin < 1e-9) {
      yMax = yMin + 1.0;
    }
  }

  if (yMin != m_effectiveYMin || yMax != m_effectiveYMax) {
    m_effectiveYMin = yMin;
    m_effectiveYMax = yMax;
    emit effectiveRangeChanged();
  }

  QVector<Quad> quads;
  const auto w = width();
  const auto h = height();

  if (w <= 0.0 || h <= 0.0) {
    std::lock_guard lock(m_quadMutex);
    m_quads = quads;
    m_quadsDirty = true;
    update();
    return;
  }

  for (const auto &entry : m_bands) {
    const auto band = entry.toMap();
    const auto top = valueToY(band.value("to").toDouble());
    const auto bottom = valueToY(band.value("from").toDouble());
    quads.push_back({QRectF(0.0, top, w, bottom - top), band.value("color").value<QColor>()});
  }

  // TODO
  // The area is one quad per sample interval down to the baseline, the line one thin quad per
  // interval; a sloped line would need triangles, and the samples arrive densely enough not to
  for (size_t index = 1; index < m_points.size(); ++index) {
    const auto &previous = m_points[index - 1];
    const auto &current = m_points[index];
    const auto x0 = timeToX(previous.atNs);
    const auto x1 = std::max(timeToX(current.atNs), x0 + 1.0);
    const auto y = valueToY(current.value);
    quads.push_back({QRectF(x0, y, x1 - x0, h - y), m_fillColor});
    quads.push_back({QRectF(x0, y - m_lineWidth / 2.0, x1 - x0, m_lineWidth), m_lineColor});
  }

  for (const auto &entry : m_guideLines) {
    const auto guide = entry.toMap();
    appendHorizontalLine(quads, 0.0, w, valueToY(guide.value("value").toDouble()), GUIDE_THICKNESS,
                         guide.value("color").value<QColor>(), guide.value("dashed").toBool());
  }

  {
    std::lock_guard lock(m_quadMutex);
    m_quads = std::move(quads);
    m_quadsDirty = true;
  }

  update();
}

qreal SeriesChartItem::valueToY(const double value) const {
  const auto range = m_effectiveYMax - m_effectiveYMin;

  if (range <= 0.0) {
    return height();
  }

  const auto fraction = std::clamp((value - m_effectiveYMin) / range, 0.0, 1.0);
  return height() - fraction * height();
}

qreal SeriesChartItem::timeToX(const int64_t atNs) const {
  const auto windowNs = m_windowSeconds * NS_PER_S;

  if (windowNs <= 0.0) {
    return width();
  }

  const auto fraction = 1.0 - static_cast<double>(m_windowEndNs - atNs) / windowNs;
  return std::clamp(fraction, 0.0, 1.0) * width();
}

} // namespace firelight::gui

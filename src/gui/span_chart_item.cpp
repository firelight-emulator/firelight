// TODO: NEEDS REVIEW
#include "span_chart_item.hpp"

#include <QHoverEvent>
#include <algorithm>
#include <cmath>

namespace firelight::gui {

namespace {
constexpr double NS_PER_MS = 1e6;
constexpr double NS_PER_S = 1e9;
constexpr qreal GUIDE_THICKNESS = 1.0;
constexpr int MAX_LANES = 6;

/** One boundary-to-boundary interval and the spans that overlapped it */
struct Bar {
  int64_t startNs = 0;
  int64_t endNs = 0;
};
} // namespace

SpanChartItem::SpanChartItem(QQuickItem *parent) : QQuickItem(parent) {
  setFlag(ItemHasContents);
  setAcceptHoverEvents(true);
}

QObject *SpanChartItem::getSource() const { return m_proxy; }

void SpanChartItem::setSource(QObject *source) {
  auto *proxy = qobject_cast<QtPerformanceMonitorProxy *>(source);

  if (m_proxy == proxy) {
    return;
  }

  if (m_proxy) {
    disconnect(m_proxy, nullptr, this, nullptr);
  }

  m_proxy = proxy;

  if (m_proxy) {
    connect(m_proxy, &QtPerformanceMonitorProxy::updated, this, &SpanChartItem::rebuild);
  }

  emit sourceChanged();
  rebuild();
}

QString SpanChartItem::getBoundaryKind() const { return m_boundaryKind; }

void SpanChartItem::setBoundaryKind(const QString &kind) {
  if (m_boundaryKind == kind) {
    return;
  }

  m_boundaryKind = kind;
  emit boundaryKindChanged();
  rebuild();
}

double SpanChartItem::getWindowSeconds() const { return m_windowSeconds; }

void SpanChartItem::setWindowSeconds(const double seconds) {
  m_windowSeconds = seconds;
  emit layoutChanged();
  rebuild();
}

double SpanChartItem::getYMaxMs() const { return m_yMaxMs; }

void SpanChartItem::setYMaxMs(const double value) {
  m_yMaxMs = value;
  emit rangeChanged();
  rebuild();
}

bool SpanChartItem::isAutoScale() const { return m_autoScale; }

void SpanChartItem::setAutoScale(const bool autoScale) {
  m_autoScale = autoScale;
  emit rangeChanged();
  rebuild();
}

double SpanChartItem::getEffectiveYMaxMs() const { return m_effectiveYMaxMs; }

double SpanChartItem::getBarWidth() const { return m_barWidth; }

void SpanChartItem::setBarWidth(const double width) {
  m_barWidth = width;
  emit layoutChanged();
  rebuild();
}

double SpanChartItem::getBarGap() const { return m_barGap; }

void SpanChartItem::setBarGap(const double gap) {
  m_barGap = gap;
  emit layoutChanged();
  rebuild();
}

QVariantList SpanChartItem::getSeriesColors() const { return m_seriesColors; }

void SpanChartItem::setSeriesColors(const QVariantList &colors) {
  m_seriesColors = colors;
  emit styleChanged();
  rebuild();
}

QColor SpanChartItem::getBarColor() const { return m_barColor; }

void SpanChartItem::setBarColor(const QColor &color) {
  m_barColor = color;
  emit styleChanged();
  rebuild();
}

QVariantList SpanChartItem::getGuideLines() const { return m_guideLines; }

void SpanChartItem::setGuideLines(const QVariantList &guideLines) {
  m_guideLines = guideLines;
  emit decorationsChanged();
  rebuild();
}

QVariantList SpanChartItem::getBands() const { return m_bands; }

void SpanChartItem::setBands(const QVariantList &bands) {
  m_bands = bands;
  emit decorationsChanged();
  rebuild();
}

QVariantMap SpanChartItem::getHoveredSpan() const { return m_hoveredSpan; }

int SpanChartItem::getBarCount() const { return m_barCount; }

QSGNode *SpanChartItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) {
  auto *node = static_cast<QSGGeometryNode *>(oldNode);
  std::lock_guard lock(m_quadMutex);

  if (!node || m_quadsDirty) {
    node = updateQuadNode(node, m_quads);
    m_quadsDirty = false;
  }

  return node;
}

void SpanChartItem::hoverMoveEvent(QHoverEvent *event) {
  const auto position = event->position();
  const Segment *best = nullptr;

  // TODO
  // The deepest segment under the pointer wins, since it is the one drawn on top
  for (const auto &segment : m_segments) {
    if (segment.rect.contains(position) && (!best || segment.depth >= best->depth)) {
      best = &segment;
    }
  }

  QVariantMap hovered;

  if (best && m_proxy) {
    hovered["kind"] = best->kind;
    hovered["offsetMs"] = best->offsetMs;
    hovered["durationMs"] = best->durationMs;
    hovered["depth"] = best->depth;
    hovered["bar"] = best->bar;
    hovered["x"] = best->rect.center().x();
    hovered["y"] = best->rect.top();

    for (const auto &kind : monitoring::Monitor::instance().getKinds()) {
      if (kind.id == best->kind) {
        hovered["name"] = QString::fromStdString(kind.name);
        hovered["summary"] = QString::fromStdString(kind.summary);
      }
    }
  }

  m_hoveredSpan = hovered;
  emit hoveredSpanChanged();
}

void SpanChartItem::hoverLeaveEvent(QHoverEvent *) {
  m_hoveredSpan.clear();
  emit hoveredSpanChanged();
}

void SpanChartItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) {
  QQuickItem::geometryChange(newGeometry, oldGeometry);
  rebuild();
}

void SpanChartItem::rebuild() {
  QVector<Quad> quads;
  m_segments.clear();
  const auto w = width();
  const auto h = height();
  const auto stride = m_barWidth + m_barGap;
  const auto barsThatFit = stride > 0.0 ? static_cast<int>(w / stride) : 0;

  if (m_proxy && w > 0.0 && h > 0.0 && barsThatFit > 0) {
    const auto &history = m_proxy->getHistory();
    const auto boundary = history.getKind(m_boundaryKind.toStdString());
    const auto &spans = history.getSpans();
    const auto windowStartNs = history.getUpdatedNs() - static_cast<int64_t>(m_windowSeconds * NS_PER_S);

    std::vector<Bar> bars;
    int64_t previousBoundaryNs = 0;

    for (const auto &span : spans) {
      if (span.kind != boundary || span.startNs < windowStartNs) {
        continue;
      }

      if (previousBoundaryNs > 0) {
        bars.push_back({previousBoundaryNs, span.startNs});
      }

      previousBoundaryNs = span.startNs;
    }

    if (static_cast<int>(bars.size()) > barsThatFit) {
      bars.erase(bars.begin(), bars.end() - barsThatFit);
    }

    auto yMaxMs = m_yMaxMs;

    if (m_autoScale) {
      yMaxMs = 1.0;

      for (const auto &bar : bars) {
        yMaxMs = std::max(yMaxMs, static_cast<double>(bar.endNs - bar.startNs) / NS_PER_MS);
      }
    }

    if (yMaxMs != m_effectiveYMaxMs) {
      m_effectiveYMaxMs = yMaxMs;
      emit effectiveRangeChanged();
    }

    const auto msToY = [h, yMaxMs](const double ms) { return h - std::clamp(ms / yMaxMs, 0.0, 1.0) * h; };

    for (const auto &entry : m_bands) {
      const auto band = entry.toMap();
      const auto top = msToY(band.value("to").toDouble());
      const auto bottom = msToY(band.value("from").toDouble());
      quads.push_back({QRectF(0.0, top, w, bottom - top), band.value("color").value<QColor>()});
    }

    // TODO
    // Bars fill from the right so the newest interval sits at the edge; each bar's height is its
    // own duration, and a span inside it is placed by when it started and how long it ran, in the
    // lane its nesting depth gives it
    const auto firstX = w - static_cast<double>(bars.size()) * stride;
    size_t spanIndex = 0;

    for (size_t barIndex = 0; barIndex < bars.size(); ++barIndex) {
      const auto &bar = bars[barIndex];
      const auto barDurationNs = static_cast<double>(bar.endNs - bar.startNs);
      const auto x = firstX + static_cast<double>(barIndex) * stride;
      const auto barTop = msToY(barDurationNs / NS_PER_MS);
      quads.push_back({QRectF(x, barTop, m_barWidth, h - barTop), m_barColor});

      while (spanIndex < spans.size() && spans[spanIndex].endNs < bar.startNs) {
        spanIndex++;
      }

      for (auto index = spanIndex; index < spans.size() && spans[index].startNs < bar.endNs; ++index) {
        const auto &span = spans[index];

        // TODO
        // Markers are the boundaries and have no length to draw; a span that is the boundary kind
        // is still drawn, so a chart sliced at frame starts shows the frames
        if (span.endNs < bar.startNs || span.endNs == span.startNs) {
          continue;
        }

        const auto clippedStartNs = std::max(span.startNs, bar.startNs);
        const auto clippedEndNs = std::min(span.endNs, bar.endNs);
        const auto lane = std::min(static_cast<int>(span.depth), MAX_LANES - 1);
        const auto laneWidth = m_barWidth / MAX_LANES;
        const auto laneX = x + lane * laneWidth;
        const auto top = msToY(static_cast<double>(clippedEndNs - bar.startNs) / NS_PER_MS);
        const auto bottom = msToY(static_cast<double>(clippedStartNs - bar.startNs) / NS_PER_MS);
        const QRectF rect(laneX, top, m_barWidth - lane * laneWidth, std::max(bottom - top, 1.0));
        quads.push_back({rect, colorForKind(span.kind)});
        m_segments.push_back({rect, span.kind, static_cast<int>(barIndex),
                              static_cast<double>(span.startNs - bar.startNs) / NS_PER_MS,
                              static_cast<double>(span.endNs - span.startNs) / NS_PER_MS, span.depth});
      }
    }

    for (const auto &entry : m_guideLines) {
      const auto guide = entry.toMap();
      appendHorizontalLine(quads, 0.0, w, msToY(guide.value("value").toDouble()), GUIDE_THICKNESS,
                           guide.value("color").value<QColor>(), guide.value("dashed").toBool());
    }

    if (static_cast<int>(bars.size()) != m_barCount) {
      m_barCount = static_cast<int>(bars.size());
      emit barCountChanged();
    }
  }

  {
    std::lock_guard lock(m_quadMutex);
    m_quads = std::move(quads);
    m_quadsDirty = true;
  }

  update();
}

QColor SpanChartItem::colorForKind(const monitoring::KindId kind) const {
  if (m_seriesColors.isEmpty() || kind == 0) {
    return QColor(255, 255, 255, 160);
  }

  return m_seriesColors[static_cast<int>((kind - 1) % static_cast<monitoring::KindId>(m_seriesColors.size()))]
      .value<QColor>();
}

} // namespace firelight::gui

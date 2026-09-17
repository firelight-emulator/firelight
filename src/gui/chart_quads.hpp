// TODO: NEEDS REVIEW
#pragma once

#include <QColor>
#include <QRectF>
#include <QSGGeometryNode>
#include <QVector>

namespace firelight::gui {

/** One filled rectangle of one colour */
struct Quad {
  QRectF rect;
  QColor color;
};

/**
 * Builds or refills a geometry node so it draws every quad as two triangles with per-vertex colour.
 * Render thread only; the quads are copied in, nothing is retained
 */
QSGGeometryNode *updateQuadNode(QSGGeometryNode *node, const QVector<Quad> &quads);

/**
 * Appends the quads for a horizontal line, dashed when asked
 */
void appendHorizontalLine(QVector<Quad> &quads, qreal x0, qreal x1, qreal y, qreal thickness, const QColor &color,
                          bool dashed);

} // namespace firelight::gui

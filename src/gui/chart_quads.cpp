// TODO: NEEDS REVIEW
#include "chart_quads.hpp"

#include <QSGVertexColorMaterial>

namespace firelight::gui {

namespace {
constexpr int VERTICES_PER_QUAD = 6;
constexpr qreal DASH_LENGTH = 6.0;
constexpr qreal DASH_GAP = 4.0;

void writeVertex(QSGGeometry::ColoredPoint2D &vertex, const qreal x, const qreal y, const QColor &color) {
  vertex.set(static_cast<float>(x), static_cast<float>(y), static_cast<uchar>(color.red()),
             static_cast<uchar>(color.green()), static_cast<uchar>(color.blue()), static_cast<uchar>(color.alpha()));
}
} // namespace

QSGGeometryNode *updateQuadNode(QSGGeometryNode *node, const QVector<Quad> &quads) {
  const auto vertexCount = static_cast<int>(quads.size()) * VERTICES_PER_QUAD;

  // TODO
  // A node the renderer first met with no vertices never draws again, so an empty chart has no node
  // and a node whose size changed is built afresh
  if (vertexCount == 0) {
    delete node;
    return nullptr;
  }

  if (node && node->geometry()->vertexCount() != vertexCount) {
    delete node;
    node = nullptr;
  }

  if (!node) {
    node = new QSGGeometryNode();
    auto *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), vertexCount);
    geometry->setDrawingMode(QSGGeometry::DrawTriangles);
    node->setGeometry(geometry);
    node->setFlag(QSGNode::OwnsGeometry);
    auto *material = new QSGVertexColorMaterial();
    material->setFlag(QSGMaterial::Blending);
    node->setMaterial(material);
    node->setFlag(QSGNode::OwnsMaterial);
  }

  auto *geometry = node->geometry();
  auto *vertices = geometry->vertexDataAsColoredPoint2D();

  for (auto index = 0; index < quads.size(); ++index) {
    const auto &quad = quads[index];
    const auto premultiplied =
        QColor::fromRgbF(quad.color.redF() * quad.color.alphaF(), quad.color.greenF() * quad.color.alphaF(),
                         quad.color.blueF() * quad.color.alphaF(), quad.color.alphaF());
    const auto left = quad.rect.left();
    const auto right = quad.rect.right();
    const auto top = quad.rect.top();
    const auto bottom = quad.rect.bottom();
    auto *vertex = vertices + index * VERTICES_PER_QUAD;
    writeVertex(vertex[0], left, top, premultiplied);
    writeVertex(vertex[1], right, top, premultiplied);
    writeVertex(vertex[2], left, bottom, premultiplied);
    writeVertex(vertex[3], right, top, premultiplied);
    writeVertex(vertex[4], right, bottom, premultiplied);
    writeVertex(vertex[5], left, bottom, premultiplied);
  }

  geometry->markVertexDataDirty();
  node->markDirty(QSGNode::DirtyGeometry);
  return node;
}

void appendHorizontalLine(QVector<Quad> &quads, const qreal x0, const qreal x1, const qreal y, const qreal thickness,
                          const QColor &color, const bool dashed) {
  if (!dashed) {
    quads.push_back({QRectF(x0, y - thickness / 2.0, x1 - x0, thickness), color});
    return;
  }

  for (auto x = x0; x < x1; x += DASH_LENGTH + DASH_GAP) {
    const auto end = std::min(x + DASH_LENGTH, x1);
    quads.push_back({QRectF(x, y - thickness / 2.0, end - x, thickness), color});
  }
}

} // namespace firelight::gui

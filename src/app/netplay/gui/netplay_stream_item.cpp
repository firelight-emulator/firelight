#include "netplay_stream_item.hpp"

#include <QQuickWindow>
#include <QSGSimpleTextureNode>

namespace firelight::gui {

NetplayStreamItem::NetplayStreamItem(QQuickItem *parent) : QQuickItem(parent) {
  setFlag(ItemHasContents);
}

void NetplayStreamItem::presentFrame(media::StreamVideoFrame frame) {
  {
    std::lock_guard lock(m_frameMutex);
    m_pendingFrame = std::move(frame); // newest frame wins
  }
  QMetaObject::invokeMethod(this, [this] { update(); }, Qt::QueuedConnection);
}

QSGNode *NetplayStreamItem::updatePaintNode(QSGNode *oldNode,
                                            UpdatePaintNodeData *) {
  auto *node = static_cast<QSGSimpleTextureNode *>(oldNode);

  std::optional<media::StreamVideoFrame> frame;
  {
    std::lock_guard lock(m_frameMutex);
    frame.swap(m_pendingFrame);
  }

  if (frame && frame->width > 0 && frame->height > 0 && window()) {
    QImage image(frame->rgba.data(), frame->width, frame->height,
                 frame->width * 4, QImage::Format_RGBA8888);
    auto *texture = window()->createTextureFromImage(image.copy());
    if (texture) {
      if (!node) {
        node = new QSGSimpleTextureNode();
        node->setOwnsTexture(true);
      }
      node->setTexture(texture);
      if (frame->width != m_frameWidth || frame->height != m_frameHeight) {
        m_frameWidth = frame->width;
        m_frameHeight = frame->height;
        QMetaObject::invokeMethod(this, [this] { emit frameSizeChanged(); },
                                  Qt::QueuedConnection);
      }
    }
  }

  if (node) {
    node->setRect(boundingRect());
  }
  return node;
}

} // namespace firelight::gui

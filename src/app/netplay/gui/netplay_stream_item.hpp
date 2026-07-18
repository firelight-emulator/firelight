#pragma once

#include <firelight/media/stream_decoder.hpp>

#include <QQuickItem>

#include <mutex>
#include <optional>

namespace firelight::gui {

// TODO
// Displays the incoming game stream: frames are handed in from decode
// threads and uploaded as scene-graph textures on the render thread
// (Not final: qmlRegisterType subclasses it.)
class NetplayStreamItem : public QQuickItem {
  Q_OBJECT
  Q_PROPERTY(int frameWidth READ frameWidth NOTIFY frameSizeChanged)
  Q_PROPERTY(int frameHeight READ frameHeight NOTIFY frameSizeChanged)

public:
  explicit NetplayStreamItem(QQuickItem *parent = nullptr);

  // Any thread
  void presentFrame(media::StreamVideoFrame frame);

  [[nodiscard]] int frameWidth() const { return m_frameWidth; }
  [[nodiscard]] int frameHeight() const { return m_frameHeight; }

signals:
  void frameSizeChanged();

protected:
  QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;

private:
  std::mutex m_frameMutex;
  std::optional<media::StreamVideoFrame> m_pendingFrame;
  int m_frameWidth = 0;
  int m_frameHeight = 0;
};

} // namespace firelight::gui

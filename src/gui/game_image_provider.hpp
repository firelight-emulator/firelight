#pragma once

#include <QQuickImageProvider>

#include <mutex>

namespace firelight::gui {
class GameImageProvider : public QQuickImageProvider {
  Q_OBJECT

public:
  GameImageProvider();

  QImage requestImage(const QString &id, QSize *size,
                      const QSize &requestedSize) override;

  void setImage(const QString &id, const QImage &image);

  QString setImage(const QImage &image);

  void removeImageWithUrl(const QString &url);

private:
  QMap<qint64, QString> m_cacheIdsToProviderIds;
  QStringList m_ids;
  QMap<QString, QImage> m_images{};
  // Guards m_images: requestImage() runs on QML's image thread while
  // setImage()/removeImageWithUrl() are called from the render and GUI threads.
  std::mutex m_mutex;
  // emulation::RewindModel *m_rewindModel;
};
} // namespace firelight::gui
// firelight

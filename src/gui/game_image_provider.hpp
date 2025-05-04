#pragma once

#include <QQuickImageProvider>

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
  // emulation::RewindModel *m_rewindModel;
};
} // namespace firelight::gui
// firelight

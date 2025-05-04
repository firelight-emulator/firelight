#include "game_image_provider.hpp"
#include <QUuid>

namespace firelight::gui {
GameImageProvider::GameImageProvider() : QQuickImageProvider(Image) {}

QImage GameImageProvider::requestImage(const QString &id, QSize *size,
                                       const QSize &requestedSize) {
  if (!m_images.contains(id)) {
    return {};
  }

  if (m_images[id].height() != 0 && m_images[id].width() != 0) {
    if (size != nullptr) {
      size->setHeight(m_images[id].height());
      size->setWidth(m_images[id].width());
    }
  }

  // if ((requestedSize.height() <= 0 && requestedSize.width() <= 0)) {
  //     return {};
  // }

  if (requestedSize.height() > 0) {
    return m_images[id].scaledToHeight(requestedSize.height(),
                                       Qt::SmoothTransformation);
  }

  if (requestedSize.width() > 0) {
    return m_images[id].scaledToWidth(requestedSize.width(),
                                      Qt::SmoothTransformation);
  }

  return m_images[id];
}

void GameImageProvider::setImage(const QString &id, const QImage &image) {
  m_images[id] = image;
}

QString GameImageProvider::setImage(const QImage &image) {
  // if (m_cacheIdsToProviderIds.contains(image.cacheKey())) {
  //     return "image://gameimages/" +
  //     m_cacheIdsToProviderIds[image.cacheKey()];
  // }

  const auto id = QUuid::createUuid().toString(QUuid::WithoutBraces);
  const auto url = "image://gameimages/" + id;

  m_images[id] = image;
  return url;
}

void GameImageProvider::removeImageWithUrl(const QString &url) {
  const auto id = url.split("/").last();

  m_images.erase(m_images.find(id));
  // if (m_images.isDetached())
  //   if (!m_images.isEmpty() && m_images.contains(id)) {
  //     // m_cacheIdsToProviderIds.remove(m_images[id].cacheKey());
  //     m_images.remove(id);
  //   }
}
} // namespace firelight::gui

#include "game_image_provider.hpp"
#include <QUuid>

namespace firelight::gui {
GameImageProvider::GameImageProvider() : QQuickImageProvider(Image) {}

QImage GameImageProvider::requestImage(const QString &id, QSize *size,
                                       const QSize &requestedSize) {
  // Copy the stored image out under the lock, then scale outside it: QImage is
  // copy-on-write, so the copy is cheap and stays valid even if another thread
  // replaces the map entry while we scale
  QImage image;
  {
    std::lock_guard lock(m_mutex);
    const auto it = m_images.constFind(id);
    if (it == m_images.constEnd()) {
      return {};
    }
    image = it.value();
  }

  if (image.height() != 0 && image.width() != 0) {
    if (size != nullptr) {
      size->setHeight(image.height());
      size->setWidth(image.width());
    }
  }

  if (requestedSize.height() > 0) {
    return image.scaledToHeight(requestedSize.height(),
                                Qt::SmoothTransformation);
  }

  if (requestedSize.width() > 0) {
    return image.scaledToWidth(requestedSize.width(),
                               Qt::SmoothTransformation);
  }

  return image;
}

void GameImageProvider::setImage(const QString &id, const QImage &image) {
  std::lock_guard lock(m_mutex);
  m_images[id] = image;
}

QString GameImageProvider::setImage(const QImage &image) {
  const auto id = QUuid::createUuid().toString(QUuid::WithoutBraces);
  const auto url = "image://gameimages/" + id;

  std::lock_guard lock(m_mutex);
  m_images[id] = image;
  return url;
}

void GameImageProvider::removeImageWithUrl(const QString &url) {
  const auto id = url.split("/").last();

  std::lock_guard lock(m_mutex);
  // erase(find(...)) is undefined behavior when the id isn't present, so only
  // erase a real hit
  if (const auto it = m_images.find(id); it != m_images.end()) {
    m_images.erase(it);
  }
}
} // namespace firelight::gui

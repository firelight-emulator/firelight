#include "game_image_provider.hpp"

namespace firelight::gui {
    GameImageProvider::GameImageProvider() : QQuickImageProvider(
        Image) {
    }

    QImage GameImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize) {
        return m_images[id];
    }

    void GameImageProvider::setImage(const QString &id, const QImage &image) {
        m_images[id] = image;
    }
} // firelight::gui

#include "game_image_provider.hpp"

namespace firelight::gui {
    GameImageProvider::GameImageProvider() : QQuickImageProvider(
        Image) {
    }

    QImage GameImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize) {
        return m_images[id];
    }

    void GameImageProvider::setImage(const QString &id, const QImage &image) {
        printf("Got image: %s\n", id.toStdString().c_str());
        // m_ids.push_back(id);
        m_images[id] = image;
        // m_images.insert(id, image);
        // m_images[id] = image.copy();
    }
} // firelight::gui

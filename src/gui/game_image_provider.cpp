#include "game_image_provider.hpp"

namespace firelight::gui {
    GameImageProvider::GameImageProvider(emulation::RewindModel *rewindModel) : QQuickImageProvider(
            Image),
        m_rewindModel(rewindModel) {
    }

    QImage GameImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize) {
        auto index = id.toInt();
        if (index >= m_rewindModel->rowCount(QModelIndex())) {
            return QImage{};
        }

        return m_rewindModel->getImageAtIndex(index);
        // // return m_items.at(index).image;
        // return QQuickImageProvider::requestImage(id, size, requestedSize);
    }
} // firelight::gui

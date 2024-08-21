#include "rewind_model.hpp"
#include <QUrl>
#include <spdlog/spdlog.h>

namespace firelight::emulation {
    QHash<int, QByteArray> RewindModel::roleNames() const {
        QHash<int, QByteArray> roles;
        roles[Id] = "id";
        roles[ImageUrl] = "image_url";
        return roles;
    }

    int RewindModel::rowCount(const QModelIndex &parent) const {
        // spdlog::info("Getting row count for rewind model");
        auto size = m_items.size() + 1;
        // spdlog::info("Size of rewind model: {}", size);
        return size;
    }

    QVariant RewindModel::data(const QModelIndex &index, int role) const {
        // spdlog::info("Getting data for rewind model before thing");
        if (role < Qt::UserRole || index.row() >= (m_items.size() + 1)) {
            return QVariant{};
        }

        auto myIndex = index.row() == 0 ? 0 : index.row() - 1;
        // auto item = index.row() == 0 ? currentSuspendPoint : m_items.at(index.row() - 1);

        // spdlog::info("Getting data for rewind model");

        switch (role) {
            case Id:
            // return item.image;
            case ImageUrl:
                return "image://gameImages/" + QString::number(myIndex);
            // return item.getImageUrl();
            default:
                return QVariant{};
        }
    }

    void RewindModel::addSuspendPoint(const SuspendPoint &suspendPoint) {
        emit beginInsertRows(QModelIndex(), 0, 0);
        m_items.prepend(suspendPoint);
        emit endInsertRows();
        spdlog::info("Added suspend point to rewind model");
    }

    void RewindModel::setCurrentImage(QImage *image) {
        currentImage = image;
        emit dataChanged(index(0), index(0));
    }

    QImage RewindModel::getImageAtIndex(int index) const {
        if (index >= m_items.size() + 1) {
            return QImage{};
        }
        if (index == 0) {
            return currentImage ? *currentImage : QImage{};
        }

        return m_items.at(index - 1).image;
    }
}

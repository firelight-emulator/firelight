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
        auto size = m_items.size();
        // spdlog::info("Size of rewind model: {}", size);
        return size;
    }

    QVariant RewindModel::data(const QModelIndex &index, int role) const {
        // spdlog::info("Getting data for rewind model before thing");
        if (role < Qt::UserRole || index.row() >= m_items.size()) {
            return QVariant{};
        }

        auto item = m_items.at(index.row());

        // spdlog::info("Getting data for rewind model");

        switch (role) {
            case Id:
                return item.image;
            case ImageUrl:
                return "image://gameImages/" + QString::number(index.row());
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

    QImage RewindModel::getImageAtIndex(int index) const {
        if (index >= m_items.size()) {
            return QImage{};
        }
        return m_items.at(index).image;
    }
}

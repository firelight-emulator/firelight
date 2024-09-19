#include "suspend_point_list_model.hpp"

#include "../game_image_provider.hpp"

namespace firelight::emulation {
    SuspendPointListModel::SuspendPointListModel(gui::GameImageProvider &imageProvider,
                                                 QObject *parent) : QAbstractListModel(parent),
                                                                    m_imageProvider(imageProvider) {
        m_items.emplace_back(Item{
            .datetime = "none",
            .locked = false,
            .hasData = false,
            .imageUrl = ""
        });
        m_items.emplace_back(Item{
            .datetime = "none",
            .locked = false,
            .hasData = false,
            .imageUrl = ""
        });
        m_items.emplace_back(Item{
            .datetime = "none",
            .locked = false,
            .hasData = false,
            .imageUrl = ""
        });
        m_items.emplace_back(Item{
            .datetime = "none",
            .locked = false,
            .hasData = false,
            .imageUrl = ""
        });
        m_items.emplace_back(Item{
            .datetime = "none",
            .locked = false,
            .hasData = false,
            .imageUrl = ""
        });
        m_items.emplace_back(Item{
            .datetime = "none",
            .locked = false,
            .hasData = false,
            .imageUrl = ""
        });
        m_items.emplace_back(Item{
            .datetime = "none",
            .locked = false,
            .hasData = false,
            .imageUrl = ""
        });
        m_items.emplace_back(Item{
            .datetime = "none",
            .locked = false,
            .hasData = false,
            .imageUrl = ""
        });
    }

    bool SuspendPointListModel::setData(const QModelIndex &index, const QVariant &value, int role) {
        printf("gonna change that there data: %d, index: %d\n", role, index.row());

        // Access the item by reference to modify it in place
        auto &item = m_items[index.row()];

        if (role == HasData) {
            item.hasData = value.toBool();
        } else if (role == Locked) {
            item.locked = !item.locked;
        }

        // Notify the view that data has changed for the given index and role
        emit suspendPointUpdated(index.row());
        emit dataChanged(index, index, {role});

        return true;
    }

    QHash<int, QByteArray> SuspendPointListModel::roleNames() const {
        QHash<int, QByteArray> roles;
        roles[Id] = "id";
        roles[Timestamp] = "timestamp";
        roles[Locked] = "locked";
        roles[ImageUrl] = "image_url";
        roles[HasData] = "has_data";
        return roles;
    }

    int SuspendPointListModel::rowCount(const QModelIndex &parent) const {
        return m_items.size();
    }

    QVariant SuspendPointListModel::data(const QModelIndex &index, int role) const {
        if (role < Qt::UserRole || index.row() >= m_items.size()) {
            return QVariant{};
        }

        const auto item = m_items.at(index.row());

        switch (role) {
            case Timestamp:
                return item.datetime;
            case Locked:
                return item.locked;
            case ImageUrl:
                return item.imageUrl;
            case HasData:
                return item.hasData;
            default:
                return QVariant{};
        }
    }

    Qt::ItemFlags SuspendPointListModel::flags(const QModelIndex &index) const {
        return Qt::ItemIsEnabled & Qt::ItemIsEditable;
    }

    void SuspendPointListModel::deleteData(const int index) {
        emit beginResetModel();
        auto item = m_items.at(index);
        item.hasData = false;

        emit endResetModel();
        emit dataChanged(createIndex(index, 0), createIndex(index, 0), {});
    }

    void SuspendPointListModel::updateData(const int index, const SuspendPoint &suspendPoint) {
        auto &item = m_items[index];
        if (!item.imageUrl.isEmpty()) {
            m_imageProvider.removeImageWithUrl(item.imageUrl);
        }

        const auto url = m_imageProvider.setImage(suspendPoint.image);

        item.locked = suspendPoint.locked;
        item.hasData = true;
        item.imageUrl = url;
        item.datetime = QDateTime::fromMSecsSinceEpoch(suspendPoint.timestamp).toString("yyyy-MM-dd hh:mm:ss");

        emit dataChanged(createIndex(index, 0), createIndex(index, 0), {});
    }

    std::optional<SuspendPointListModel::Item> SuspendPointListModel::getItem(const int index) {
        if (index > m_items.size()) {
            return std::nullopt;
        }

        return {m_items[index]};
    }
}

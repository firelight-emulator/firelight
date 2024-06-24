#pragma once

#include <QAbstractListModel>
#include <firelight/db/platform.hpp>

namespace firelight::gui {
    class PlatformListModel : public QAbstractListModel {
        Q_OBJECT

    public:
        PlatformListModel();

        [[nodiscard]] int rowCount(const QModelIndex &parent) const override;

        [[nodiscard]] QVariant data(const QModelIndex &index,
                                    int role) const override;

        [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    private:
        enum Roles { PlatformId = Qt::UserRole + 1, DisplayName, IconUrl, Buttons, Sticks };

        struct Item {
            int platformId;
            QString displayName;
            QString iconUrl;
            QVector<QJsonObject> buttons;
            QVector<QJsonObject> sticks;
        };

        QVector<Item> m_items;
    };
} // firelight::gui

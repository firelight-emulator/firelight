#pragma once
#include <QAbstractListModel>

#include "../../app/saves/suspend_point.hpp"
#include "../../gui/game_image_provider.hpp"


namespace firelight::emulation {
    static constexpr int MAX_NUM_SUSPEND_POINTS = 8;

    class SuspendPointListModel final : public QAbstractListModel {
        Q_OBJECT

    public:
        struct Item {
            QString datetime;
            bool locked;
            QString imageUrl;
            bool hasData;
        };

        /**
         * @enum Roles
         * @brief The roles that can be used with this model.
         */
        enum Roles {
            Id = Qt::UserRole + 1,
            Index,
            Timestamp,
            Locked,
            ImageUrl,
            HasData
        };

        explicit SuspendPointListModel(gui::GameImageProvider &imageProvider, QObject *parent = nullptr);

        bool setData(const QModelIndex &index, const QVariant &value, int role) override;

        [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

        [[nodiscard]] int rowCount(const QModelIndex &parent) const override;

        [[nodiscard]] QVariant data(const QModelIndex &index,
                                    int role) const override;

        [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;

        void deleteData(int index);

        void updateData(int index, const SuspendPoint &suspendPoint);

        std::optional<Item> getItem(int index);

    signals:
        void suspendPointUpdated(int index);

    private:
        gui::GameImageProvider &m_imageProvider;
        QList<Item> m_items;
    };
} // firelight::emulation

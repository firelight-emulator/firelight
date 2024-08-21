#pragma once
#include <QAbstractListModel>
#include <QQuickImageProvider>

#include "../../app/saves/suspend_point.hpp"

namespace firelight::emulation {
    class RewindModel final : public QAbstractListModel {
        Q_OBJECT

    public:
        /**
         * @enum Roles
         * @brief The roles that can be used with this model.
         */
        enum Roles {
            Id = Qt::UserRole + 1,
            Index,
            ImageUrl,
        };

        [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

        [[nodiscard]] int rowCount(const QModelIndex &parent) const override;

        [[nodiscard]] QVariant data(const QModelIndex &index,
                                    int role) const override;

        void addSuspendPoint(const SuspendPoint &suspendPoint);

        void setCurrentImage(QImage *image);

        QImage getImageAtIndex(int index) const;

    private:
        // db::IContentDatabase &m_contentDatabase;
        QImage *currentImage = nullptr;
        QList<SuspendPoint> m_items;
    };
} // firelight::emulation

#pragma once
#include <QAbstractListModel>
#include <firelight/libretro/retropad.hpp>

class PlatformInputListModel : public QAbstractListModel {
    Q_OBJECT

public:
    PlatformInputListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    QHash<int, QByteArray> roleNames() const override;

private:
    struct Item {
        firelight::libretro::IRetroPad::Input button;
        QString iconUrl;
        QString displayName;
    };

    enum Roles { ButtonId = Qt::UserRole + 1, IconUrl, DisplayName };

    QVector<Item> m_items;
};

#pragma once
#include <QAbstractListModel>
#include <QUrl>
#include <qsettings.h>
#include "../user_library.hpp"

namespace firelight::gui {
    class LibraryPathModel : public QAbstractListModel {
        Q_OBJECT

        std::vector<library::WatchedDirectory> m_items;
        std::unique_ptr<QSettings> m_settings;
        library::IUserLibrary &m_userLibrary;

    public:
        enum Roles { Path = Qt::UserRole + 1, LocalFilename, NumGameFiles };

        explicit LibraryPathModel(library::IUserLibrary &userLibrary);

        int rowCount(const QModelIndex &parent) const override;

        QVariant data(const QModelIndex &index, int role) const override;

        bool setData(const QModelIndex &index, const QVariant &value,
                     int role) override;

        QHash<int, QByteArray> roleNames() const override;

        Qt::ItemFlags flags(const QModelIndex &index) const override;
    };
} // namespace firelight::gui

#pragma once
#include <QAbstractListModel>
#include <qsettings.h>
#include <QUrl>
#include <firelight/library_database.hpp>

namespace firelight::gui {
    class LibraryPathModel : public QAbstractListModel {
        Q_OBJECT

        std::vector<db::LibraryContentDirectory> m_items;
        std::unique_ptr<QSettings> m_settings;
        const db::ILibraryDatabase &m_libraryDatabase;

    public:
        enum Roles {
            Path = Qt::UserRole + 1,
            LocalFilename,
            NumGameFiles
        };

        LibraryPathModel(const db::ILibraryDatabase &libraryDatabase);

        int rowCount(const QModelIndex &parent) const override;

        QVariant data(const QModelIndex &index, int role) const override;

        bool setData(const QModelIndex &index, const QVariant &value, int role) override;

        QHash<int, QByteArray> roleNames() const override;

        Qt::ItemFlags flags(const QModelIndex &index) const override;
    };
} // namespace firelight::gui

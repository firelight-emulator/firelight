#pragma once
#include <QAbstractListModel>
#include <firelight/library/entry.hpp>
#include <firelight/library/user_library.hpp>


namespace firelight::library {
    class EntryListModel final : public QAbstractListModel {
        Q_OBJECT

    public:
        /**
         * @enum Roles
         * @brief The roles that can be used with this model.
         */
        enum Roles {
            Id = Qt::UserRole + 1,
            DisplayName,
            ContentHash,
            PlatformId,
            ActiveSaveSlot,
            Hidden,
            Icon1x1SourceUrl,
            BoxartFrontSourceUrl,
            BoxartBackSourceUrl,
            Description,
            ReleaseYear,
            Developer,
            Publisher,
            Genres,
            RegionIds,
            CreatedAt
        };

        explicit EntryListModel(IUserLibrary &userLibrary, QObject *parent = nullptr);

        [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

        [[nodiscard]] int rowCount(const QModelIndex &parent) const override;

        [[nodiscard]] QVariant data(const QModelIndex &index,
                                    int role) const override;

        void reset();

    private:
        IUserLibrary &m_userLibrary;
        QList<Entry> m_items{};
    };
} // firelight::emulation

#include "entry_list_model.hpp"

namespace firelight::library {
    EntryListModel::EntryListModel(IUserLibrary &userLibrary, QObject *parent) : QAbstractListModel(parent),
        m_userLibrary(userLibrary) {
        for (const auto &entry: m_userLibrary.getEntries(0, 0)) {
            m_items.emplace_back(entry);
        }

        // m_items.emplace_back(Entry{
        //     .displayName = "Super Mario 64",
        //     .contentHash = "none",
        //     .platformId = 0,
        //     .activeSaveSlot = 1,
        //     .hidden = false,
        //     .icon1x1SourceUrl = "https://media.retroachievements.org/Images/047942.png",
        //     .boxartFrontSourceUrl = "none",
        //     .boxartBackSourceUrl = "none",
        //     .description = "none",
        //     .releaseYear = 0,
        //     .developer = "none",
        //     .publisher = "none",
        //     .genres = "none",
        //     .regionIds = "none",
        //     .createdAt = 0
        // });
        // m_items.emplace_back(Entry{
        //     .displayName = "Paper Mario",
        //     .contentHash = "none",
        //     .platformId = 0,
        //     .activeSaveSlot = 1,
        //     .hidden = false,
        //     .icon1x1SourceUrl = "https://media.retroachievements.org/Images/041428.png",
        //     .boxartFrontSourceUrl = "none",
        //     .boxartBackSourceUrl = "none",
        //     .description = "none",
        //     .releaseYear = 0,
        //     .developer = "none",
        //     .publisher = "none",
        //     .genres = "none",
        //     .regionIds = "none",
        //     .createdAt = 0
        // });
        // m_items.emplace_back(Entry{
        //     .displayName = "Sonic the Hedgehog",
        //     .contentHash = "none",
        //     .platformId = 0,
        //     .activeSaveSlot = 1,
        //     .hidden = false,
        //     .icon1x1SourceUrl = "https://media.retroachievements.org/Images/085573.png",
        //     .boxartFrontSourceUrl = "none",
        //     .boxartBackSourceUrl = "none",
        //     .description = "none",
        //     .releaseYear = 0,
        //     .developer = "none",
        //     .publisher = "none",
        //     .genres = "none",
        //     .regionIds = "none",
        //     .createdAt = 0
        // });
        // m_items.emplace_back(Entry{
        //     .displayName = "Sonic the Hedgehog 2",
        //     .contentHash = "none",
        //     .platformId = 0,
        //     .activeSaveSlot = 1,
        //     .hidden = false,
        //     .icon1x1SourceUrl = "https://media.retroachievements.org/Images/085632.png",
        //     .boxartFrontSourceUrl = "none",
        //     .boxartBackSourceUrl = "none",
        //     .description = "none",
        //     .releaseYear = 0,
        //     .developer = "none",
        //     .publisher = "none",
        //     .genres = "none",
        //     .regionIds = "none",
        //     .createdAt = 0
        // });
        // m_items.emplace_back(Entry{
        //     .displayName = "Pokémon Red Version",
        //     .contentHash = "none",
        //     .platformId = 0,
        //     .activeSaveSlot = 1,
        //     .hidden = false,
        //     .icon1x1SourceUrl = "https://media.retroachievements.org/Images/092935.png",
        //     .boxartFrontSourceUrl = "none",
        //     .boxartBackSourceUrl = "none",
        //     .description = "none",
        //     .releaseYear = 0,
        //     .developer = "none",
        //     .publisher = "none",
        //     .genres = "none",
        //     .regionIds = "none",
        //     .createdAt = 0
        // });
        // m_items.emplace_back(Entry{
        //     .displayName = "Pokémon Gold Version",
        //     .contentHash = "none",
        //     .platformId = 0,
        //     .activeSaveSlot = 1,
        //     .hidden = false,
        //     .icon1x1SourceUrl = "https://media.retroachievements.org/Images/034452.png",
        //     .boxartFrontSourceUrl = "none",
        //     .boxartBackSourceUrl = "none",
        //     .description = "none",
        //     .releaseYear = 0,
        //     .developer = "none",
        //     .publisher = "none",
        //     .genres = "none",
        //     .regionIds = "none",
        //     .createdAt = 0
        // });
        // m_items.emplace_back(Entry{
        //     .displayName = "Pokémon Ruby Version",
        //     .contentHash = "none",
        //     .platformId = 0,
        //     .activeSaveSlot = 1,
        //     .hidden = false,
        //     .icon1x1SourceUrl = "https://media.retroachievements.org/Images/034448.png",
        //     .boxartFrontSourceUrl = "none",
        //     .boxartBackSourceUrl = "none",
        //     .description = "none",
        //     .releaseYear = 0,
        //     .developer = "none",
        //     .publisher = "none",
        //     .genres = "none",
        //     .regionIds = "none",
        //     .createdAt = 0
        // });
    }

    QHash<int, QByteArray> EntryListModel::roleNames() const {
        QHash<int, QByteArray> roles;
        roles[Id] = "id";
        roles[DisplayName] = "displayName";
        roles[ContentHash] = "contentHash";
        roles[PlatformId] = "platformId";
        roles[ActiveSaveSlot] = "activeSaveSlot";
        roles[Hidden] = "hidden";
        roles[Icon1x1SourceUrl] = "icon1x1SourceUrl";
        roles[BoxartFrontSourceUrl] = "boxartFrontSourceUrl";
        roles[BoxartBackSourceUrl] = "boxartBackSourceUrl";
        roles[Description] = "description";
        roles[ReleaseYear] = "releaseYear";
        roles[Developer] = "developer";
        roles[Publisher] = "publisher";
        roles[Genres] = "genres";
        roles[RegionIds] = "regionIds";
        roles[CreatedAt] = "createdAt";
        return roles;
    }

    int EntryListModel::rowCount(const QModelIndex &parent) const {
        return m_items.size();
    }

    QVariant EntryListModel::data(const QModelIndex &index, int role) const {
        if (role < Qt::UserRole || index.row() >= m_items.size()) {
            return QVariant{};
        }

        const auto item = m_items.at(index.row());

        switch (role) {
            case Id:
                return item.id;
            case DisplayName:
                return item.displayName;
            case ContentHash:
                return item.contentHash;
            case PlatformId:
                return item.platformId;
            case ActiveSaveSlot:
                return item.activeSaveSlot;
            case Hidden:
                return item.hidden;
            case Icon1x1SourceUrl:
                return item.icon1x1SourceUrl;
            case BoxartFrontSourceUrl:
                return item.boxartFrontSourceUrl;
            case BoxartBackSourceUrl:
                return item.boxartBackSourceUrl;
            case Description:
                return item.description;
            case ReleaseYear:
                return item.releaseYear;
            case Developer:
                return item.developer;
            case Publisher:
                return item.publisher;
            case Genres:
                return item.genres;
            case RegionIds:
                return item.regionIds;
            case CreatedAt:
                return item.createdAt;
            default:
                return QVariant{};
        }
    }

    void EntryListModel::reset() {
        emit beginResetModel();
        m_items.clear();
        for (const auto &entry: m_userLibrary.getEntries(0, 0)) {
            m_items.emplace_back(entry);
        }
        emit endResetModel();
    }
}

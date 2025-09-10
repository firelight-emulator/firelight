#include "entry_list_model.hpp"

#include "platforms/platform_service.hpp"

#include <emulation/emulation_service.hpp>
#include <spdlog/spdlog.h>

namespace firelight::library {
EntryListModel::EntryListModel(IUserLibrary &userLibrary, QObject *parent)
    : QAbstractListModel(parent), m_userLibrary(userLibrary) {
  m_gamePlayedConnection =
      EventDispatcher::instance().subscribe<emulation::EmulationStartedEvent>(
          [this](const emulation::EmulationStartedEvent &event) {
            for (auto i = 0; i < m_items.size(); ++i) {
              auto &item = m_items[i];
              if (item.entry.contentHash == event.contentHash) {
                item.lastPlayedEpochMillis =
                    QDateTime::currentMSecsSinceEpoch();
                emit dataChanged(createIndex(i, 0), createIndex(i, 0),
                                 {LastPlayedAt});
              }
            }
          });
  reset();
}

QHash<int, QByteArray> EntryListModel::roleNames() const {
  QHash<int, QByteArray> roles;
  // roles[Qt::DisplayRole] = "displayName";
  roles[Id] = "entryId";
  roles[DisplayName] = "displayName";
  roles[ContentHash] = "contentHash";
  roles[PlatformId] = "platformId";
  roles[PlatformIconName] = "platformIconName";
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
  roles[FolderIds] = "folderIds";
  roles[CreatedAt] = "createdAt";
  roles[LastPlayedAt] = "lastPlayedAt";
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
    return item.entry.id;
  case DisplayName:
    return item.entry.displayName;
  case ContentHash:
    return item.entry.contentHash;
  case PlatformId:
    return item.entry.platformId;
  case PlatformIconName: {
    auto platform = platforms::PlatformService::getInstance().getPlatform(
        item.entry.platformId);
    if (!platform.has_value()) {
      return {};
    }

    return QString::fromStdString(platform.value().slug);
  }
  case ActiveSaveSlot:
    return item.entry.activeSaveSlot;
  case Hidden:
    return item.entry.hidden;
  case Icon1x1SourceUrl:
    return item.entry.icon1x1SourceUrl;
  case BoxartFrontSourceUrl:
    return item.entry.boxartFrontSourceUrl;
  case BoxartBackSourceUrl:
    return item.entry.boxartBackSourceUrl;
  case Description:
    return item.entry.description;
  case ReleaseYear:
    return item.entry.releaseYear;
  case Developer:
    return item.entry.developer;
  case Publisher:
    return item.entry.publisher;
  case Genres:
    return item.entry.genres;
  case RegionIds:
    return item.entry.regionIds;
  case CreatedAt:
    return item.entry.createdAt;
  case FolderIds:
    return QVariant::fromValue(item.entry.folderIds);
  case LastPlayedAt:
    return item.lastPlayedEpochMillis;
  default:
    return QVariant{};
  }
}
bool EntryListModel::setData(const QModelIndex &index, const QVariant &value,
                             int role) {
  if (index.row() < 0 || index.row() >= m_items.size()) {
    return false;
  }

  auto &item = m_items[index.row()];

  switch (role) {
  case DisplayName:
    item.entry.displayName = value.toString();
    break;
  default:
    return false;
  }

  m_userLibrary.update(item.entry);

  return true;
}

Qt::ItemFlags EntryListModel::flags(const QModelIndex &index) const {
  return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
}

void EntryListModel::addEntryToFolder(int entryId, int folderId) {
  auto libraryEntryInfo =
      FolderEntryInfo{.folderId = folderId, .entryId = entryId};
  if (!m_userLibrary.create(libraryEntryInfo)) {
    spdlog::error("Failed to add entry {} to folder {}", entryId, folderId);
    return;
  }

  for (auto &item : m_items) {
    if (item.entry.id == entryId) {
      if (std::ranges::find(item.entry.folderIds, folderId) !=
          item.entry.folderIds.end()) {
        spdlog::warn("Entry {} is already in folder {}", entryId, folderId);
        return;
      }

      beginResetModel();
      item.entry.folderIds.push_back(folderId);
      endResetModel();
      break;
    }
  }
}

void EntryListModel::removeEntryFromFolder(int entryId, int folderId) {
  if (auto entryInfo =
          FolderEntryInfo{.folderId = folderId, .entryId = entryId};
      !m_userLibrary.deleteFolderEntry(entryInfo)) {
    spdlog::error("Failed to remove entry {} from folder {}", entryId,
                  folderId);
    return;
  }

  for (auto &item : m_items) {
    if (item.entry.id == entryId) {
      auto it = std::ranges::find(item.entry.folderIds, folderId);
      if (it != item.entry.folderIds.end()) {
        beginResetModel();
        item.entry.folderIds.erase(it);
        endResetModel();
        break;
      }

      spdlog::warn("Entry {} is not in folder {}", entryId, folderId);
      break;
    }
  }
}

void EntryListModel::reset() {
  emit beginResetModel();
  m_items.clear();
  auto activityLog = getActivityLog();
  for (const auto &entry : m_userLibrary.getEntries(0, 0)) {
    if (!entry.hidden) {
      auto item = Item{.entry = entry};

      auto session =
          activityLog->getLatestPlaySession(entry.contentHash.toStdString());
      if (session) {
        item.lastPlayedEpochMillis = session->endTime;
      }

      m_items.emplace_back(item);
    }
  }
  emit endResetModel();
}

void EntryListModel::removeFolderId(int folderId) {
  beginResetModel();
  for (auto &item : m_items) {
    auto it = std::ranges::find(item.entry.folderIds, folderId);
    if (it != item.entry.folderIds.end()) {
      item.entry.folderIds.erase(it);
    }
  }
  endResetModel();
}
} // namespace firelight::library

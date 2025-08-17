#include "entry_list_model.hpp"

#include "platforms/platform_service.hpp"

#include <spdlog/spdlog.h>

namespace firelight::library {
EntryListModel::EntryListModel(IUserLibrary &userLibrary, QObject *parent)
    : QAbstractListModel(parent), m_userLibrary(userLibrary) {
  for (const auto &entry : m_userLibrary.getEntries(0, 0)) {
    if (!entry.hidden) {
      m_items.emplace_back(entry);
    }
  }
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
  case PlatformIconName: {
    auto platform =
        platforms::PlatformService::getInstance().getPlatform(item.platformId);
    if (!platform.has_value()) {
      return {};
    }

    return QString::fromStdString(platform.value().slug);
  }
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
  case FolderIds:
    return QVariant::fromValue(item.folderIds);
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
    item.displayName = value.toString();
    break;
  default:
    return false;
  }

  m_userLibrary.update(item);

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
    if (item.id == entryId) {
      if (std::ranges::find(item.folderIds, folderId) != item.folderIds.end()) {
        spdlog::warn("Entry {} is already in folder {}", entryId, folderId);
        return;
      }

      beginResetModel();
      item.folderIds.push_back(folderId);
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
    if (item.id == entryId) {
      auto it = std::ranges::find(item.folderIds, folderId);
      if (it != item.folderIds.end()) {
        beginResetModel();
        item.folderIds.erase(it);
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
  for (const auto &entry : m_userLibrary.getEntries(0, 0)) {
    if (!entry.hidden) {
      m_items.emplace_back(entry);
    }
  }
  emit endResetModel();
}

void EntryListModel::removeFolderId(int folderId) {
  beginResetModel();
  for (auto &item : m_items) {
    auto it = std::ranges::find(item.folderIds, folderId);
    if (it != item.folderIds.end()) {
      item.folderIds.erase(it);
    }
  }
  endResetModel();
}
} // namespace firelight::library

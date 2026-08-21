#include "playlist_item_model.hpp"

#include <firelight/library/user_library_service.hpp>

#include <spdlog/spdlog.h>

namespace firelight::gui {
bool LibraryFolderListModel::setData(const QModelIndex &index, const QVariant &value, int role) {
  if (index.row() < 0 || index.row() >= m_items.size()) {
    return false;
  }

  library::FolderInfo &item = m_items[index.row()];

  switch (role) {
  case DisplayName:
    item.displayName = value.toString().toStdString();
    break;
  case Description:
    item.description = value.toString().toStdString();
    break;
  case Icon1x1SourceUrl:
    item.iconSourceUrl = value.toString().toStdString();
    break;
  case Color:
    item.color = value.toString().toStdString();
    break;
  case FilterJson:
    item.filterJson = value.toString().toStdString();
    break;
  case SortRole:
    item.sortRole = value.toString().toStdString();
    break;
  case SortAscending:
    item.sortAscending = value.toBool();
    break;
  default:
    return false;
  }

  getLibraryService()->update(item);

  emit dataChanged(index, index, {role});

  return true;
}

Qt::ItemFlags LibraryFolderListModel::flags(const QModelIndex &index) const {
  return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
}

LibraryFolderListModel::LibraryFolderListModel() { m_items = getLibraryService()->listFolders(); }

int LibraryFolderListModel::rowCount(const QModelIndex &parent) const { return m_items.size(); }

QVariant LibraryFolderListModel::data(const QModelIndex &index, int role) const {
  if (role < Qt::UserRole || index.row() >= m_items.size()) {
    return QVariant{};
  }

  auto item = m_items.at(index.row());

  switch (role) {
  case FolderId:
    return item.id;
  case DisplayName:
    return QString::fromStdString(item.displayName);
  case Description:
    return QString::fromStdString(item.description);
  case Icon1x1SourceUrl:
    return QString::fromStdString(item.iconSourceUrl);
  case FolderType:
    return item.type;
  case FilterJson:
    return QString::fromStdString(item.filterJson);
  case Color:
    return QString::fromStdString(item.color);
  case SortRole:
    return QString::fromStdString(item.sortRole);
  case SortAscending:
    return item.sortAscending;
  case ParentId:
    return item.parentId;
  case Position:
    return item.position;
  default:
    return QVariant{};
  }
}

QHash<int, QByteArray> LibraryFolderListModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[FolderId] = "folderId";
  roles[DisplayName] = "displayName";
  roles[Description] = "description";
  roles[Icon1x1SourceUrl] = "icon1x1SourceUrl";
  roles[FolderType] = "folderType";
  roles[FilterJson] = "filterJson";
  roles[Color] = "color";
  roles[SortRole] = "sortRole";
  roles[SortAscending] = "sortAscending";
  roles[ParentId] = "parentId";
  roles[Position] = "position";
  return roles;
}

bool LibraryFolderListModel::addFolder(const QString &displayName) {
  if (auto folder = library::FolderInfo{.displayName = displayName.toStdString()};
      getLibraryService()->create(folder)) {
    beginInsertRows(QModelIndex(), rowCount(QModelIndex()), rowCount(QModelIndex()));

    m_items.push_back(folder);
    endInsertRows();

    return true;
  }

  return false;
}

int LibraryFolderListModel::createFolder(const QString &displayName, const int parentId) {
  auto folder = library::FolderInfo{.displayName = displayName.toStdString()};
  if (!getLibraryService()->create(folder)) {
    return -1;
  }
  if (parentId >= 0) {
    getLibraryService()->setFolderParent(folder.id, parentId);
  }
  // Reordering under a parent changes positions; re-pull the ordered list
  beginResetModel();
  m_items = getLibraryService()->listFolders();
  endResetModel();
  return folder.id;
}

int LibraryFolderListModel::addSmartFolder(const QString &displayName, const QString &filterJson) {
  auto folder = library::FolderInfo{.displayName = displayName.toStdString(),
                                    .type = static_cast<int>(library::FolderType::Smart),
                                    .filterJson = filterJson.toStdString()};
  if (!getLibraryService()->create(folder)) {
    return -1;
  }

  beginInsertRows(QModelIndex(), rowCount(QModelIndex()), rowCount(QModelIndex()));
  m_items.push_back(folder);
  endInsertRows();
  return folder.id;
}

bool LibraryFolderListModel::updateSmartFolder(const int folderId, const QString &filterJson) {
  for (int i = 0; i < m_items.size(); ++i) {
    if (m_items[i].id != folderId) {
      continue;
    }

    m_items[i].type = static_cast<int>(library::FolderType::Smart);
    m_items[i].filterJson = filterJson.toStdString();
    if (!getLibraryService()->update(m_items[i])) {
      spdlog::warn("Failed to update smart folder with ID {}", folderId);
      return false;
    }

    const auto idx = index(i, 0);
    emit dataChanged(idx, idx, {FilterJson, FolderType});
    return true;
  }

  spdlog::warn("No folder with ID {} to update", folderId);
  return false;
}

bool LibraryFolderListModel::setFolderColor(const int folderId, const QString &color) {
  for (int i = 0; i < m_items.size(); ++i) {
    if (m_items[i].id != folderId) {
      continue;
    }
    m_items[i].color = color.toStdString();
    if (!getLibraryService()->update(m_items[i])) {
      spdlog::warn("Failed to set color of folder {}", folderId);
      return false;
    }
    const auto idx = index(i, 0);
    emit dataChanged(idx, idx, {Color});
    return true;
  }
  return false;
}

bool LibraryFolderListModel::setFolderName(const int folderId, const QString &displayName) {
  for (int i = 0; i < m_items.size(); ++i) {
    if (m_items[i].id != folderId) {
      continue;
    }
    m_items[i].displayName = displayName.toStdString();
    if (!getLibraryService()->update(m_items[i])) {
      spdlog::warn("Failed to rename folder {}", folderId);
      return false;
    }
    const auto idx = index(i, 0);
    emit dataChanged(idx, idx, {DisplayName});
    return true;
  }
  return false;
}

bool LibraryFolderListModel::setFolderSort(const int folderId, const QString &sortRole, const bool ascending) {
  for (int i = 0; i < m_items.size(); ++i) {
    if (m_items[i].id != folderId) {
      continue;
    }
    m_items[i].sortRole = sortRole.toStdString();
    m_items[i].sortAscending = ascending;
    if (!getLibraryService()->update(m_items[i])) {
      spdlog::warn("Failed to set sort of folder {}", folderId);
      return false;
    }
    const auto idx = index(i, 0);
    emit dataChanged(idx, idx, {SortRole, SortAscending});
    return true;
  }
  return false;
}

bool LibraryFolderListModel::reorderFolders(const int parentId, const QVariantList &orderedIds) {
  std::vector<int> ids;
  ids.reserve(orderedIds.size());
  for (const auto &v : orderedIds) {
    ids.push_back(v.toInt());
  }

  if (!getLibraryService()->reorderFolders(parentId, ids)) {
    return false;
  }

  // Positions changed; re-pull the ordered list so the model reflects it
  beginResetModel();
  m_items = getLibraryService()->listFolders();
  endResetModel();
  return true;
}

bool LibraryFolderListModel::setFolderParent(const int folderId, const int newParentId) {
  if (!getLibraryService()->setFolderParent(folderId, newParentId)) {
    return false;
  }

  beginResetModel();
  m_items = getLibraryService()->listFolders();
  endResetModel();
  return true;
}

void LibraryFolderListModel::deleteFolder(const int folderId) {
  if (!getLibraryService()->deleteFolder(folderId)) {
    spdlog::warn("Failed to delete folder with ID {}", folderId);
    return;
  }

  for (int i = 0; i < m_items.size(); ++i) {
    if (m_items[i].id == folderId) {
      beginRemoveRows(QModelIndex(), i, i);
      m_items.erase(m_items.begin() + i);
      endRemoveRows();
      emit folderDeleted(folderId);
      break;
    }
  }
}
} // namespace firelight::gui

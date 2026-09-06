#include "playlist_item_model.hpp"

#include "gui/models/library_entry_sort_filter_model.hpp"

#include <firelight/library/user_library_service.hpp>

#include <algorithm>
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

bool LibraryFolderListModel::moveRows(const QModelIndex &sourceParent, const int sourceRow, const int count,
                                      const QModelIndex &destinationParent, const int destinationChild) {
  const auto rows = static_cast<int>(m_items.size());

  if (sourceRow < 0 || count <= 0 || sourceRow + count > rows || destinationChild < 0 || destinationChild > rows) {
    return false;
  }

  if (destinationChild >= sourceRow && destinationChild <= sourceRow + count) {
    return false;
  }

  const auto parentId = m_items[sourceRow].parentId;
  const auto spanBegin = std::min(sourceRow, destinationChild);
  const auto spanEnd = std::max(sourceRow + count, destinationChild);

  for (auto i = spanBegin; i < spanEnd; ++i) {
    if (m_items[i].parentId != parentId) {
      return false;
    }
  }

  if (!beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationChild)) {
    return false;
  }

  const auto block = m_items.begin() + sourceRow;

  if (destinationChild > sourceRow) {
    std::rotate(block, block + count, m_items.begin() + destinationChild);
  } else {
    std::rotate(m_items.begin() + destinationChild, block, block + count);
  }

  endMoveRows();

  std::vector<int> ordered;
  for (const auto &item : m_items) {
    if (item.parentId == parentId) {
      ordered.push_back(item.id);
    }
  }

  if (!getLibraryService()->reorderFolders(parentId, ordered)) {
    spdlog::error("Failed to persist folder order for parent {}", parentId);
    return false;
  }

  return true;
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

const library::FolderInfo *LibraryFolderListModel::findFolder(const int folderId) const {
  for (const auto &item : m_items) {
    if (item.id == folderId) {
      return &item;
    }
  }

  return nullptr;
}

QVariantMap LibraryFolderListModel::folderById(const int folderId) const {
  for (const auto &item : m_items) {
    if (item.id != folderId) {
      continue;
    }

    return QVariantMap{{"folderId", item.id},
                       {"displayName", QString::fromStdString(item.displayName)},
                       {"description", QString::fromStdString(item.description)},
                       {"icon1x1SourceUrl", QString::fromStdString(item.iconSourceUrl)},
                       {"folderType", item.type},
                       {"filterJson", QString::fromStdString(item.filterJson)},
                       {"color", QString::fromStdString(item.color)},
                       {"sortRole", QString::fromStdString(item.sortRole)},
                       {"sortAscending", item.sortAscending}};
  }

  return {};
}

QVariantList LibraryFolderListModel::getManualFolders() const {
  QVariantList folders;

  for (const auto &item : m_items) {
    if (item.type == static_cast<int>(library::FolderType::Manual)) {
      folders.append(QVariantMap{{"folderId", item.id}, {"displayName", QString::fromStdString(item.displayName)}});
    }
  }

  return folders;
}

QVariantList LibraryFolderListModel::foldersInParent(const int parentId) const {
  QVariantList folders;

  for (const auto &item : m_items) {
    if (item.parentId != parentId) {
      continue;
    }

    folders.append(QVariantMap{{"folderId", item.id},
                               {"displayName", QString::fromStdString(item.displayName)},
                               {"icon1x1SourceUrl", QString::fromStdString(item.iconSourceUrl)},
                               {"color", QString::fromStdString(item.color)},
                               {"folderType", item.type}});
  }

  return folders;
}

int LibraryFolderListModel::getCount() const { return static_cast<int>(m_items.size()); }

QVariantList LibraryFolderListModel::getSortOptions() { return LibraryEntrySortFilterModel::getSortOptions(); }

bool LibraryFolderListModel::addFolder(const QString &displayName) {
  if (auto folder = library::FolderInfo{.displayName = displayName.toStdString()};
      getLibraryService()->create(folder)) {
    beginInsertRows(QModelIndex(), rowCount(QModelIndex()), rowCount(QModelIndex()));

    m_items.push_back(folder);
    endInsertRows();
    emit countChanged();

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

  beginResetModel();
  m_items = getLibraryService()->listFolders();
  endResetModel();
  emit countChanged();

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
  emit countChanged();
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

  beginResetModel();
  m_items = getLibraryService()->listFolders();
  endResetModel();
  emit countChanged();

  return true;
}

bool LibraryFolderListModel::setFolderParent(const int folderId, const int newParentId) {
  if (!getLibraryService()->setFolderParent(folderId, newParentId)) {
    return false;
  }

  beginResetModel();
  m_items = getLibraryService()->listFolders();
  endResetModel();
  emit countChanged();

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
      emit countChanged();
      emit folderDeleted(folderId);
      break;
    }
  }
}
} // namespace firelight::gui

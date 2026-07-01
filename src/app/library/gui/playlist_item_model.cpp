#include "playlist_item_model.hpp"
#include <firelight/library/user_library_service.hpp>

namespace firelight::gui {
  bool LibraryFolderListModel::setData(const QModelIndex &index,
                                       const QVariant &value, int role) {
    if (index.row() < 0 || index.row() >= m_items.size())
      return false;

    library::FolderInfo &item = m_items[index.row()];

    switch (role) {
      case FolderId:
        return false;
      case DisplayName:
        item.displayName = value.toString().toStdString();
      // TODO: Persist to db
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

  LibraryFolderListModel::LibraryFolderListModel() {
    m_items = getLibraryService()->listFolders();
  }

  int LibraryFolderListModel::rowCount(const QModelIndex &parent) const {
    return m_items.size();
  }

  QVariant LibraryFolderListModel::data(const QModelIndex &index,
                                        int role) const {
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
    return roles;
  }

  bool LibraryFolderListModel::addFolder(const QString &displayName) {
    if (auto folder =
          library::FolderInfo{.displayName = displayName.toStdString()};
      getLibraryService()->create(folder)) {
      beginInsertRows(QModelIndex(), rowCount(QModelIndex()),
                      rowCount(QModelIndex()));

      m_items.push_back(folder);
      endInsertRows();

      return true;
    }

    return false;
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

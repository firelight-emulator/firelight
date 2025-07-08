#include "content_directory_model.hpp"

namespace firelight::gui {

bool ContentDirectoryModel::setData(const QModelIndex &index,
                                    const QVariant &value, int role) {
  if (index.row() < 0 || index.row() >= m_items.size())
    return false;

  library::WatchedDirectory &item = m_items[index.row()];

  switch (role) {
  case DirectoryId:
    return false;
  case Path:
    item.path = value.toString();
    // TODO: Persist to db
    break;
  default:
    return false;
  }

  m_library.update(item);

  emit dataChanged(index, index, {role});

  return true;
}

Qt::ItemFlags ContentDirectoryModel::flags(const QModelIndex &index) const {
  return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
}

void ContentDirectoryModel::deleteItem(const int id) {
  for (auto i = 0; i < m_items.size(); ++i) {
    if (m_items[i].id == id) {
      spdlog::info("Deleting content directory with ID: {}", id);
      beginRemoveRows(QModelIndex{}, i, i);
      m_library.deleteContentDirectory(id);
      m_items.erase(m_items.begin() + i);
      endRemoveRows();
      return;
    }
  }
}
void ContentDirectoryModel::addItem(QString path) {
  library::WatchedDirectory item;
  item.path = std::move(path);

  if (!m_library.create(item)) {
    return;
  }

  beginInsertRows(QModelIndex{}, m_items.size(), m_items.size());
  m_items.push_back(item);
  endInsertRows();
}

ContentDirectoryModel::ContentDirectoryModel(library::IUserLibrary &library)
    : m_library(library) {
  m_items = m_library.getWatchedDirectories();
}

int ContentDirectoryModel::rowCount(const QModelIndex &parent) const {
  return m_items.size();
}

QVariant ContentDirectoryModel::data(const QModelIndex &index, int role) const {
  if (role < Qt::UserRole || index.row() >= m_items.size()) {
    return QVariant{};
  }

  auto item = m_items.at(index.row());

  switch (role) {
  case DirectoryId:
    return item.id;
  case Path:
    return item.path;
  default:
    return QVariant{};
  }
}

QHash<int, QByteArray> ContentDirectoryModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[DirectoryId] = "directory_id";
  roles[Path] = "path";
  return roles;
}

} // namespace firelight::gui
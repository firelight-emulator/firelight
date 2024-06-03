#include "library_path_model.hpp"

namespace firelight::gui {
LibraryPathModel::LibraryPathModel(db::ILibraryDatabase &libraryDatabase)
    : m_libraryDatabase(libraryDatabase) {
  // m_settings = std::make_unique<QSettings>();
  m_items = m_libraryDatabase.getAllLibraryContentDirectories();
}

int LibraryPathModel::rowCount(const QModelIndex &parent) const {
  return m_items.size();
}

QVariant LibraryPathModel::data(const QModelIndex &index, int role) const {
  if (role < Qt::UserRole || index.row() >= m_items.size()) {
    return {};
  }

  auto item = m_items.at(index.row());

  switch (role) {
  case LocalFilename: {
    auto t = QString::fromStdString(item.path);
    auto val = QUrl::fromUserInput(t);
    return val;
  }
  case Path: {
    return QString::fromStdString(item.path);
  }
  case NumGameFiles:
    return item.numGameFiles;
  default:
    return QVariant{};
  }
}

bool LibraryPathModel::setData(const QModelIndex &index, const QVariant &value,
                               int role) {
  if (role < Qt::UserRole || index.row() >= m_items.size()) {
    return false;
  }

  auto &item = m_items.at(index.row());
  switch (role) {
  case Path:
    item.path = QUrl(value.toString()).toLocalFile().toStdString();
    m_libraryDatabase.updateLibraryContentDirectory(item);

    emit dataChanged(index, index, {Path, LocalFilename});
    return true;
  case NumGameFiles:
    item.numGameFiles = value.toInt();

    emit dataChanged(index, index, {role});
    return true;
  default:
    return false;
  }
}

QHash<int, QByteArray> LibraryPathModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[Path] = "path";
  roles[LocalFilename] = "local_filename";
  roles[NumGameFiles] = "num_game_files";
  return roles;
}

Qt::ItemFlags LibraryPathModel::flags(const QModelIndex &index) const {
  return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
}
} // namespace firelight::gui

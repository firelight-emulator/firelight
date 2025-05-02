#include "library_path_model.hpp"

#include "../user_library.hpp"

namespace firelight::gui {
  LibraryPathModel::LibraryPathModel(library::IUserLibrary &userLibrary)
    : m_userLibrary(userLibrary) {
    // m_settings = std::make_unique<QSettings>();
    m_items = m_userLibrary.getWatchedDirectories();
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
        auto val = QUrl::fromUserInput(item.path);
        return val;
      }
      case Path: {
        return item.path;
      }
      case NumGameFiles:
        return item.numContentFiles;
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
        item.path = QUrl(value.toString()).toLocalFile();
        m_userLibrary.updateWatchedDirectory(item);

        emit dataChanged(index, index, {Path, LocalFilename});
        return true;
      case NumGameFiles:
        item.numContentFiles = value.toInt();

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

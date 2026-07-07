#include "capture_list_model.hpp"

#include <QFile>
#include <QUrl>

namespace firelight::gui {

CaptureListModel::CaptureListModel(media::IGameCaptureRepository &captures,
                                   library::UserLibraryService &library,
                                   QObject *parent)
    : QAbstractListModel(parent), m_captures(captures), m_library(library) {
  refresh();
}

QHash<int, QByteArray> CaptureListModel::roleNames() const {
  return {
      {CaptureId, "captureId"},
      {GameContentHash, "gameContentHash"},
      {GameName, "gameName"},
      {GameIconUrl, "gameIconUrl"},
      {CaptureTypeName, "captureType"},
      {FilePath, "filePath"},
      {FileUrl, "fileUrl"},
      {ThumbnailUrl, "thumbnailUrl"},
      {Timestamp, "timestamp"},
      {Favorite, "favorite"},
  };
}

int CaptureListModel::rowCount(const QModelIndex &parent) const {
  return parent.isValid() ? 0 : static_cast<int>(m_items.size());
}

QVariant CaptureListModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() >= m_items.size()) {
    return {};
  }
  const auto &c = m_items.at(index.row());
  switch (role) {
  case CaptureId:
    return c.id;
  case GameContentHash:
    return QString::fromStdString(c.contentHash);
  case GameName:
    return gameInfoFor(c.contentHash).name;
  case GameIconUrl:
    return gameInfoFor(c.contentHash).iconUrl;
  case CaptureTypeName:
    return c.type == media::CaptureType::Clip ? QStringLiteral("clip")
                                              : QStringLiteral("screenshot");
  case FilePath:
    return QString::fromStdString(c.filePath);
  case FileUrl:
    return QUrl::fromLocalFile(QString::fromStdString(c.filePath)).toString();
  case ThumbnailUrl: {
    const auto &thumb = c.thumbnailPath.empty() ? c.filePath : c.thumbnailPath;
    return QUrl::fromLocalFile(QString::fromStdString(thumb)).toString();
  }
  case Timestamp:
    return static_cast<qint64>(c.timestamp);
  case Favorite:
    return c.favorite;
  default:
    return {};
  }
}

bool CaptureListModel::setData(const QModelIndex &index, const QVariant &value,
                               int role) {
  if (!index.isValid() || index.row() >= m_items.size() || role != Favorite) {
    return false;
  }
  auto &c = m_items[index.row()];
  c.favorite = value.toBool();
  m_captures.setFavorite(c.id, c.favorite);
  emit dataChanged(index, index, {Favorite});
  return true;
}

void CaptureListModel::refresh() {
  beginResetModel();
  m_items.clear();
  for (const auto &c : m_captures.listAll()) {
    m_items.append(c);
  }
  m_gameInfoCache.clear();
  endResetModel();
  emit countChanged();
}

void CaptureListModel::toggleFavorite(int captureId) {
  for (int i = 0; i < m_items.size(); ++i) {
    if (m_items[i].id == captureId) {
      setData(index(i, 0), !m_items[i].favorite, Favorite);
      return;
    }
  }
}

void CaptureListModel::removeCapture(int captureId) {
  for (int i = 0; i < m_items.size(); ++i) {
    if (m_items[i].id != captureId) {
      continue;
    }
    const auto c = m_items[i];
    m_captures.remove(captureId);
    QFile::remove(QString::fromStdString(c.filePath));
    if (c.type == media::CaptureType::Clip && !c.thumbnailPath.empty()) {
      QFile::remove(QString::fromStdString(c.thumbnailPath));
    }
    beginRemoveRows(QModelIndex(), i, i);
    m_items.removeAt(i);
    endRemoveRows();
    emit countChanged();
    return;
  }
}

const CaptureListModel::GameInfo &
CaptureListModel::gameInfoFor(const std::string &contentHash) const {
  if (const auto it = m_gameInfoCache.find(contentHash);
      it != m_gameInfoCache.end()) {
    return it->second;
  }
  GameInfo info;
  if (const auto entry = m_library.getEntryWithContentHash(contentHash)) {
    info.name = QString::fromStdString(entry->displayName);
    info.iconUrl = QString::fromStdString(entry->icon1x1SourceUrl);
  }
  return m_gameInfoCache.emplace(contentHash, std::move(info)).first->second;
}

} // namespace firelight::gui

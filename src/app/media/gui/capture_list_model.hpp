#pragma once

#include <firelight/library/user_library_service.hpp>
#include <firelight/media/game_capture_repository.hpp>

#include <QAbstractListModel>
#include <QList>

#include <string>
#include <unordered_map>

namespace firelight::gui {

// Exposes the capture index (screenshots + clips) to the QML gallery. A flat
// list; filtering/sorting/grouping-by-game is done in QML with a
// SortFilterProxyModel over these roles
class CaptureListModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(int count READ getCount NOTIFY countChanged)

public:
  enum Roles {
    CaptureId = Qt::UserRole + 1,
    GameContentHash,
    GameName,
    GameIconUrl,
    CaptureTypeName, // "screenshot" | "clip"
    FilePath,
    FileUrl,      // file:// url to the .png/.mp4 (viewer/player)
    ThumbnailUrl, // file:// url to the grid thumbnail
    Timestamp,
    Favorite,
  };

  CaptureListModel(media::IGameCaptureRepository &captures,
                   library::UserLibraryService &library,
                   QObject *parent = nullptr);

  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index,
                              int role) const override;
  bool setData(const QModelIndex &index, const QVariant &value,
               int role) override;

  // Reloads from the repository (e.g. when the gallery opens)
  Q_INVOKABLE void refresh();
  Q_INVOKABLE void toggleFavorite(int captureId);
  // Removes the capture from the index and deletes its file(s)
  Q_INVOKABLE void removeCapture(int captureId);

  [[nodiscard]] int getCount() const {
    return static_cast<int>(m_items.size());
  }

signals:
  void countChanged();

private:
  struct GameInfo {
    QString name;
    QString iconUrl;
  };
  const GameInfo &gameInfoFor(const std::string &contentHash) const;

  media::IGameCaptureRepository &m_captures;
  library::UserLibraryService &m_library;
  QList<media::GameCapture> m_items;
  mutable std::unordered_map<std::string, GameInfo> m_gameInfoCache;
};

} // namespace firelight::gui

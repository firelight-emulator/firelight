#pragma once
#include "../play_session.hpp"
#include <QAbstractListModel>

namespace firelight::activity {
class PlaySessionListModel : public QAbstractListModel {
  Q_OBJECT

public:
  enum Roles {
    StartTime = Qt::UserRole + 1,
    EndTime,
    NumUnpausedSeconds,
    SaveSlotNumber
  };

  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;

  [[nodiscard]] QVariant data(const QModelIndex &index,
                              int role) const override;

  void refreshItems(std::vector<PlaySession> &playSessions);

private:
  QVector<PlaySession> m_items{};
};
} // namespace firelight::activity

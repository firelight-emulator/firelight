#pragma once

#include "achievements/models/achievement.hpp"
#include "achievements/models/user_unlock.hpp"

#include <QAbstractListModel>

namespace firelight::gui {
class AchievementListModel : public QAbstractListModel {
  Q_OBJECT

public:
  struct Item {
    achievements::Achievement achievement;
    achievements::UserUnlock unlockState;
  };

  explicit AchievementListModel(const QVector<Item> &items,
                                QObject *parent = nullptr);

  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index,
                              int role) const override;

  int size() const;

  void setHardcore(bool hardcore);

  enum Roles {
    Id = Qt::UserRole + 1,
    Name,
    Description,
    ImageUrl,
    Earned,
    EarnedDate,
    EarnedTimestamp,
    Points,
    DisplayOrder,
    Type
  };

private:
  QVector<Item> m_items;
  bool m_hardcore = false;
};
} // namespace firelight::gui

#pragma once

#include "achievements/models/achievement.hpp"
#include "achievements/models/user_unlock.hpp"

#include <QAbstractListModel>

namespace firelight::gui {
class AchievementListModel : public QAbstractListModel {
  Q_OBJECT

public:
  explicit AchievementListModel(
      std::vector<achievements::Achievement> achievements,
      QObject *parent = nullptr);

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex &parent) const override;

  [[nodiscard]] QVariant data(const QModelIndex &index,
                              int role) const override;

  Q_INVOKABLE void setSortType(const QString &sortType);

  int size() const;

  enum Roles {
    Id = Qt::UserRole + 1,
    Name,
    Description,
    ImageUrl,
    EarnedHardcore,
    Earned,
    EarnedDateHardcore,
    EarnedDate,
    Points,
    DisplayOrder,
    Type
  };

  struct Item {
    achievements::Achievement achievement;
    achievements::UserUnlock unlockState;
  };

private:
  QVector<Item> m_achievements;
};
} // namespace firelight::gui

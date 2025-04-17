#pragma once

#include <QAbstractListModel>
#include "achievements/achievement.hpp"

namespace firelight::gui {
  class AchievementListModel : public QAbstractListModel {
    Q_OBJECT

  public:
    explicit AchievementListModel(QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;

    int rowCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    Q_INVOKABLE void refreshAchievements(const QVector<achievements::Achievement> &achievements);

    Q_INVOKABLE void setSortType(const QString &sortType);

    int size() const;

    enum Roles {
      Id = Qt::UserRole + 1, Name, Description, ImageUrl, EarnedHardcore, Earned, EarnedDateHardcore, EarnedDate,
      Points, DisplayOrder, Type
    };

  private:
    QVector<achievements::Achievement> m_achievements;
  };
} // firelight::gui


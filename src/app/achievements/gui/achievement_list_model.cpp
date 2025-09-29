#include "achievement_list_model.hpp"

#include <QDateTime>
#include <utility>

namespace firelight::gui {
AchievementListModel::AchievementListModel(const QVector<Item> &items,
                                           QObject *parent)
    : QAbstractListModel(parent) {
  m_items = items;
}

QHash<int, QByteArray> AchievementListModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[Id] = "achievementId";
  roles[Name] = "name";
  roles[Description] = "description";
  roles[ImageUrl] = "iconUrl";
  roles[Earned] = "earned";
  roles[EarnedDate] = "earnedDate";
  roles[EarnedTimestamp] = "earnedTimestamp";
  roles[Points] = "points";
  roles[Type] = "type";
  roles[DisplayOrder] = "displayOrder";
  return roles;
}

int AchievementListModel::rowCount(const QModelIndex &parent) const {
  return m_items.size();
}

QVariant AchievementListModel::data(const QModelIndex &index, int role) const {
  if (role < Qt::UserRole || index.row() >= m_items.size()) {
    return QVariant{};
  }

  const auto item = m_items.at(index.row());

  switch (role) {
  case Id:
    return QVariant::fromValue(item.achievement.id);
  case Name:
    return QVariant::fromValue(QString::fromStdString(item.achievement.name));
  case Description:
    return QVariant::fromValue(
        QString::fromStdString(item.achievement.description));
  case ImageUrl:
    return QVariant::fromValue(
        QString::fromStdString(item.achievement.imageUrl));
  case Earned:
    return QVariant::fromValue(m_hardcore ? item.unlockState.earnedHardcore
                                          : item.unlockState.earned);
  case EarnedDate:
    return QVariant::fromValue(
        QDateTime::fromSecsSinceEpoch(
            m_hardcore ? item.unlockState.unlockTimestampHardcore
                       : item.unlockState.unlockTimestamp)
            .toString("MM/dd/yyyy hh:mm ap"));
  case EarnedTimestamp:
    return QVariant::fromValue(m_hardcore
                                   ? item.unlockState.unlockTimestampHardcore
                                   : item.unlockState.unlockTimestamp);
  case Points:
    return QVariant::fromValue(item.achievement.points);
  case Type: {
    if (item.achievement.type == "missable") {
      return QVariant::fromValue(QString::fromStdString("Missable"));
    }

    if (item.achievement.type == "progression") {
      return QVariant::fromValue(QString::fromStdString("Progression"));
    }

    if (item.achievement.type == "win_condition") {
      return QVariant::fromValue(QString::fromStdString("Win condition"));
    }

    return QVariant::fromValue(QString::fromStdString(item.achievement.type));
  }
  case DisplayOrder:
    return QVariant::fromValue(item.achievement.displayOrder);
  default:
    return {};
  }
}

int AchievementListModel::size() const { return m_items.size(); }
void AchievementListModel::setHardcore(const bool hardcore) {
  emit beginResetModel();
  m_hardcore = hardcore;
  emit endResetModel();
}
} // namespace firelight::gui

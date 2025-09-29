#include "achievement_list_sort_filter_model.hpp"

#include "achievement_list_model.hpp"

namespace firelight::gui {
AchievementListSortFilterModel::AchievementListSortFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent) {
  setDynamicSortFilter(true);
}

QString AchievementListSortFilterModel::sortMethod() const {
  switch (m_sortType) {
  case SortType::AToZ:
    return "a-z";
  case SortType::ZToA:
    return "z-a";
  case SortType::EarnedDate:
    return "earned_date";
  case SortType::PointsMost:
    return "points_most";
  case SortType::PointsLeast:
    return "points_least";
  case SortType::Type:
    return "type";
  case SortType::Default:
  default:
    return "default";
  }
}

void AchievementListSortFilterModel::setSortMethod(const QString &sortType) {
  SortType newSortType;
  if (sortType == "a-z") {
    newSortType = SortType::AToZ;
  } else if (sortType == "z-a") {
    newSortType = SortType::ZToA;
  } else if (sortType == "earned_date") {
    newSortType = SortType::EarnedDate;
  } else if (sortType == "points_most") {
    newSortType = SortType::PointsMost;
  } else if (sortType == "points_least") {
    newSortType = SortType::PointsLeast;
  } else if (sortType == "default") {
    newSortType = SortType::Default;
  } else if (sortType == "type") {
    newSortType = SortType::Type;
  } else {
    return;
  }

  if (newSortType != m_sortType) {
    m_sortType = newSortType;
    invalidate();
    sort(0);
    emit sortMethodChanged();
  }
}

bool AchievementListSortFilterModel::filterAcceptsRow(
    int source_row, const QModelIndex &sourceParent) const {
  return QSortFilterProxyModel::filterAcceptsRow(source_row, sourceParent);
}

bool AchievementListSortFilterModel::lessThan(
    const QModelIndex &source_left, const QModelIndex &source_right) const {
  if (m_sortType == SortType::AToZ) {
    const auto left = sourceModel()
                          ->data(source_left, AchievementListModel::Roles::Name)
                          .toString();
    const auto right =
        sourceModel()
            ->data(source_right, AchievementListModel::Roles::Name)
            .toString();
    return left < right;
  }
  if (m_sortType == SortType::ZToA) {
    const auto left = sourceModel()
                          ->data(source_left, AchievementListModel::Roles::Name)
                          .toString();
    const auto right =
        sourceModel()
            ->data(source_right, AchievementListModel::Roles::Name)
            .toString();
    return left > right;
  }
  if (m_sortType == SortType::Type) {
    const auto left = sourceModel()
                          ->data(source_left, AchievementListModel::Roles::Type)
                          .toString();
    const auto right =
        sourceModel()
            ->data(source_right, AchievementListModel::Roles::Type)
            .toString();
    return left > right;
  }
  if (m_sortType == SortType::EarnedDate) {
    const auto left =
        sourceModel()
            ->data(source_left, AchievementListModel::Roles::EarnedTimestamp)
            .toULongLong();
    const auto right =
        sourceModel()
            ->data(source_right, AchievementListModel::Roles::EarnedTimestamp)
            .toULongLong();
    // Make sure unearned go to the bottom of the list
    if (left == 0 && right == 0) {
      return true;
    }
    if (left == 0) {
      return false;
    }
    if (right == 0) {
      return true;
    }

    return left < right;
  }
  if (m_sortType == SortType::PointsMost) {
    const auto left =
        sourceModel()
            ->data(source_left, AchievementListModel::Roles::Points)
            .toInt();
    const auto right =
        sourceModel()
            ->data(source_right, AchievementListModel::Roles::Points)
            .toInt();
    return left > right;
  }

  if (m_sortType == SortType::PointsLeast) {
    const auto left =
        sourceModel()
            ->data(source_left, AchievementListModel::Roles::Points)
            .toInt();
    const auto right =
        sourceModel()
            ->data(source_right, AchievementListModel::Roles::Points)
            .toInt();
    return left < right;
  }

  if (m_sortType == SortType::Default) {
    const auto left =
        sourceModel()
            ->data(source_left, AchievementListModel::Roles::DisplayOrder)
            .toInt();
    const auto right =
        sourceModel()
            ->data(source_right, AchievementListModel::Roles::DisplayOrder)
            .toInt();
    return left < right;
  }

  return false;
}
} // namespace firelight::gui

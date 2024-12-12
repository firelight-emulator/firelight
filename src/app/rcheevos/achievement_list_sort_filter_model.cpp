#include "achievement_list_sort_filter_model.hpp"

#include "achievement_list_model.hpp"


namespace firelight::gui {
    AchievementListSortFilterModel::AchievementListSortFilterModel(QObject *parent) : QSortFilterProxyModel(parent) {
    }

    QString AchievementListSortFilterModel::sortType() const {
        switch (m_sortType) {
            case SortType::Title:
                return "title";
            case SortType::EarnedDate:
                return "earned_date";
            case SortType::Points:
                return "points";
            case SortType::Default:
            default:
                return "default";
        }
    }

    void AchievementListSortFilterModel::setSortType(const QString &sortType) {
        SortType newSortType;
        if (sortType == "title") {
            newSortType = SortType::Title;
        } else if (sortType == "earned_date") {
            newSortType = SortType::EarnedDate;
        } else if (sortType == "points") {
            newSortType = SortType::Points;
        } else if (sortType == "default") {
            newSortType = SortType::Default;
        } else {
            return;
        }

        if (newSortType != m_sortType) {
            m_sortType = newSortType;
            invalidate();
            sort(0);
            emit sortTypeChanged();
        }
    }

    bool AchievementListSortFilterModel::filterAcceptsRow(int source_row, const QModelIndex &sourceParent) const {
        return QSortFilterProxyModel::filterAcceptsRow(source_row, sourceParent);
    }

    bool AchievementListSortFilterModel::lessThan(const QModelIndex &source_left,
                                                  const QModelIndex &source_right) const {
        if (m_sortType == SortType::Title) {
            const auto left = sourceModel()->data(source_left, AchievementListModel::Roles::Name).toString();
            const auto right = sourceModel()->data(source_right, AchievementListModel::Roles::Name).toString();
            return left < right;
        }
        if (m_sortType == SortType::EarnedDate) {
            const auto left = sourceModel()->data(source_left, AchievementListModel::Roles::EarnedDateHardcore).
                    toString();
            const auto right = sourceModel()->data(source_right, AchievementListModel::Roles::EarnedDateHardcore).
                    toString();
            // Make sure unearned go to the bottom of the list
            if (left.isEmpty() && right.isEmpty()) {
                return true;
            }
            if (left.isEmpty()) {
                return false;
            }
            if (right.isEmpty()) {
                return true;
            }

            return left < right;
        }
        if (m_sortType == SortType::Points) {
            const auto left = sourceModel()->data(source_left, AchievementListModel::Roles::Points).toInt();
            const auto right = sourceModel()->data(source_right, AchievementListModel::Roles::Points).toInt();
            return left < right;
        }

        if (m_sortType == SortType::Default) {
            const auto left = sourceModel()->data(source_left, AchievementListModel::Roles::DisplayOrder).toInt();
            const auto right = sourceModel()->data(source_right, AchievementListModel::Roles::DisplayOrder).toInt();
            return left < right;
        }

        return false;
    }
} // firelight::gui

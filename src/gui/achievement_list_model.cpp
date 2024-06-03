#include "achievement_list_model.hpp"

#include <utility>

namespace firelight::gui {
    AchievementListModel::AchievementListModel(QObject *parent) : QAbstractListModel(parent) {
    }

    QHash<int, QByteArray> AchievementListModel::roleNames() const {
        QHash<int, QByteArray> roles;
        roles[Id] = "achievement_id";
        roles[Name] = "name";
        roles[Description] = "description";
        roles[ImageUrl] = "image_url";
        roles[EarnedHardcore] = "earned_hardcore";
        roles[Earned] = "earned";
        roles[EarnedDateHardcore] = "earned_date_hardcore";
        roles[EarnedDate] = "earned_date";
        roles[Points] = "points";
        roles[DisplayOrder] = "display_order";
        return roles;
    }

    int AchievementListModel::rowCount(const QModelIndex &parent) const {
        return m_achievements.size();
    }

    QVariant AchievementListModel::data(const QModelIndex &index, int role) const {
        if (role < Qt::UserRole || index.row() >= m_achievements.size()) {
            return QVariant{};
        }

        const auto item = m_achievements.at(index.row());

        switch (role) {
            case Id:
                return item.id;
            case Name:
                return QString::fromStdString(item.name);
            case Description:
                return QString::fromStdString(item.description);
            case ImageUrl:
                return QString::fromStdString(item.imageUrl);
            case EarnedHardcore:
                return item.earnedHardcore;
            case Earned:
                return item.earned;
            case EarnedDateHardcore:
                return QString::fromStdString(item.earnedDateHardcore);
            case EarnedDate:
                return QString::fromStdString(item.earnedDate);
            case Points:
                return item.points;
            case DisplayOrder:
                return item.displayOrder;
            default:
                return {};
        }
    }

    void AchievementListModel::refreshAchievements(const QVector<achievements::Achievement> &achievements) {
        printf("REFRESHING\n");
        emit beginResetModel();
        m_achievements.clear();
        for (const auto &achievement: achievements) {
            m_achievements.push_back(achievement);
        }

        emit endResetModel();
        printf("DONE REFRESHING\n");
    }

    void AchievementListModel::setSortType(const QString &sortType) {
        printf("AchievementListModel::setSortType: %s\n", sortType.toStdString().c_str());
    }

    int AchievementListModel::size() const {
        return m_achievements.size();
    }
} // firelight::gui

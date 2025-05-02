#pragma once

#include <QSortFilterProxyModel>

namespace firelight::gui {
    class AchievementListSortFilterModel final : public QSortFilterProxyModel {
        Q_OBJECT
        Q_PROPERTY(
            QString sortType READ sortType WRITE setSortType NOTIFY sortTypeChanged)

    public:
        explicit AchievementListSortFilterModel(QObject *parent = nullptr);

        [[nodiscard]] QString sortType() const;

        void setSortType(const QString &sortType);

    signals:
        void sortTypeChanged();

    protected:
        [[nodiscard]] bool filterAcceptsRow(int source_row,
                                            const QModelIndex &sourceParent) const override;

        [[nodiscard]] bool lessThan(const QModelIndex &source_left,
                                    const QModelIndex &source_right) const override;

    private:
        enum class SortType { Default, Title, EarnedDate, Points };

        enum class Mode { Hardcore, Softcore, Both };

        SortType m_sortType = SortType::Default;
    };
} // firelight::gui

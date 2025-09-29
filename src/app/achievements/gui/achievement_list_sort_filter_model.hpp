#pragma once

#include <QSortFilterProxyModel>

namespace firelight::gui {
class AchievementListSortFilterModel final : public QSortFilterProxyModel {
  Q_OBJECT
  Q_PROPERTY(QString sortMethod READ sortMethod WRITE setSortMethod NOTIFY
                 sortMethodChanged)

public:
  explicit AchievementListSortFilterModel(QObject *parent = nullptr);

  [[nodiscard]] QString sortMethod() const;

  void setSortMethod(const QString &sortType);

signals:
  void sortMethodChanged();

protected:
  [[nodiscard]] bool
  filterAcceptsRow(int source_row,
                   const QModelIndex &sourceParent) const override;

  [[nodiscard]] bool lessThan(const QModelIndex &source_left,
                              const QModelIndex &source_right) const override;

private:
  enum class SortType {
    Default,
    AToZ,
    ZToA,
    EarnedDate,
    PointsMost,
    PointsLeast,
    Type,
    NotEarned
  };

  enum class Mode { Hardcore, Softcore, Both };

  SortType m_sortType = SortType::Default;
};
} // namespace firelight::gui

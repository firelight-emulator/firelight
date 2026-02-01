#pragma once
#include "achievement_list_model.hpp"
#include "achievement_list_sort_filter_model.hpp"
#include "achievements/models/achievement_set.hpp"
#include "service_accessor.hpp"

#include <QObject>

namespace firelight::achievements {
class AchievementSetItem : public QObject, public ServiceAccessor {
  Q_OBJECT
  Q_PROPERTY(int setId MEMBER m_setId NOTIFY setIdChanged)
  Q_PROPERTY(
      bool hardcore READ isHardcore WRITE setHardcore NOTIFY hardcoreChanged)
  Q_PROPERTY(QString iconUrl MEMBER m_iconUrl NOTIFY setIdChanged)
  Q_PROPERTY(bool hasAchievements MEMBER m_hasAchievements NOTIFY setIdChanged)
  Q_PROPERTY(QString name MEMBER m_setName NOTIFY setIdChanged)
  Q_PROPERTY(int numEarned MEMBER m_numEarned NOTIFY setIdChanged)
  Q_PROPERTY(
      int numEarnedHardcore MEMBER m_numEarnedHardcore NOTIFY setIdChanged)
  Q_PROPERTY(int numAchievements MEMBER m_numAchievements NOTIFY setIdChanged)
  Q_PROPERTY(int totalNumPoints MEMBER m_totalNumPoints NOTIFY setIdChanged)
  Q_PROPERTY(QSortFilterProxyModel *achievements READ getAchievements NOTIFY
                 setIdChanged)

public:
  explicit AchievementSetItem(const AchievementSet &set,
                              QObject *parent = nullptr);

  bool isHardcore() const { return m_hardcore; }
  void setHardcore(const bool hardcore) {
    if (m_hardcore == hardcore) {
      return;
    }
    m_hardcore = hardcore;

    if (m_achievementListModel) {
      m_achievementListModel->setHardcore(m_hardcore);
    }

    emit hardcoreChanged();
  }

  [[nodiscard]] gui::AchievementListSortFilterModel *getAchievements() const;

signals:
  void setIdChanged();
  void hardcoreChanged();

private:
  bool m_hasAchievements = false;
  unsigned m_setId = 0;
  QString m_setName = "lol";
  QString m_iconUrl;
  unsigned m_numEarned = 0;
  unsigned m_numEarnedHardcore = 0;
  unsigned m_numAchievements = 0;
  unsigned m_totalNumPoints = 0;

  bool m_hardcore = false;

  std::unique_ptr<gui::AchievementListSortFilterModel> m_sortFilterModel;
  std::unique_ptr<gui::AchievementListModel> m_achievementListModel;
};

} // namespace firelight::achievements
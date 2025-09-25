#pragma once
#include "achievement_list_model.hpp"
#include "service_accessor.hpp"

#include <QObject>
#include <manager_accessor.hpp>

namespace firelight::achievements {
class AchievementSetItem : public QObject, public ServiceAccessor {
  Q_OBJECT
  Q_PROPERTY(int setId MEMBER m_setId NOTIFY contentHashChanged)
  Q_PROPERTY(QString contentHash READ getContentHash WRITE setContentHash NOTIFY
                 contentHashChanged)
  Q_PROPERTY(QString iconUrl MEMBER m_iconUrl NOTIFY contentHashChanged)
  Q_PROPERTY(
      bool hasAchievements MEMBER m_hasAchievements NOTIFY contentHashChanged)
  Q_PROPERTY(QString name MEMBER m_setName NOTIFY contentHashChanged)
  Q_PROPERTY(int numEarned MEMBER m_numEarned NOTIFY contentHashChanged)
  Q_PROPERTY(
      int numAchievements MEMBER m_numAchievements NOTIFY contentHashChanged)
  Q_PROPERTY(int numAchievementsEarned MEMBER m_numAchievementsEarned NOTIFY
                 contentHashChanged)
  Q_PROPERTY(
      int totalNumPoints MEMBER m_totalNumPoints NOTIFY contentHashChanged)

public:
  explicit AchievementSetItem(QObject *parent = nullptr) : QObject(parent) {};

  [[nodiscard]] QString getContentHash() const;
  void setContentHash(const QString &contentHash);

  [[nodiscard]] gui::AchievementListModel *getAchievements() const;

signals:
  void contentHashChanged();

private:
  bool m_hasAchievements = false;
  int m_setId = 0;
  QString m_contentHash;
  QString m_setName = "lol";
  QString m_iconUrl;
  int m_numEarned = 0;
  int m_numAchievements = 0;
  int m_numAchievementsEarned = 0;
  int m_totalNumPoints = 0;

  // gui::AchievementListModel m_achievementListModel;
};

} // namespace firelight::achievements
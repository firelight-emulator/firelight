#pragma once
#include "achievement_list_model.hpp"
#include <QObject>

namespace firelight::achievements {
class AchievementSetItem : public QObject {
  Q_OBJECT
  Q_PROPERTY(int setId READ getSetId WRITE setSetId NOTIFY setIdChanged)
  Q_PROPERTY(QString contentHash READ getContentHash WRITE setContentHash NOTIFY contentHashChanged)
  Q_PROPERTY(QString name READ getSetName NOTIFY setNameChanged)
  Q_PROPERTY(int numAchievements READ getNumAchievements NOTIFY achievementsChanged)
  Q_PROPERTY(int totalNumPoints READ getTotalNumPoints NOTIFY totalNumPointsChanged)
  Q_PROPERTY(QAbstractListModel *achievements READ getAchievements NOTIFY achievementsChanged)

public:
  [[nodiscard]] QString getContentHash() const;
  void setContentHash(const QString &contentHash);
  [[nodiscard]] int getNumAchievements() const;
  [[nodiscard]] int getTotalNumPoints() const;

  [[nodiscard]] QString getSetName() const;
  void setSetId(int setId);
  [[nodiscard]] int getSetId() const;

  [[nodiscard]] gui::AchievementListModel *getAchievements() const;

signals:
  void contentHashChanged();
  void totalNumPointsChanged();
  void setNameChanged();
  void achievementsChanged();
  void setIdChanged();

private:
  int m_setId = 0;
  QString m_contentHash;
  QString m_setName = "lol";
  int m_numAchievements = 0;
  int m_totalNumPoints = 0;

  gui::AchievementListModel m_achievementListModel;

  QVariantList m_achievements;
  // platform?
  // List of achievements
  // name of set
  // user's progress
};

}
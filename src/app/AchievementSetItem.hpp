#pragma once
#include <QObject>

namespace firelight::achievements {
class AchievementSetItem : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString contentHash READ getContentHash WRITE setContentHash NOTIFY contentHashChanged)
  Q_PROPERTY(QString name READ getSetName NOTIFY setNameChanged)
  Q_PROPERTY(int numAchievements READ getNumAchievements NOTIFY numAchievementsChanged)
  Q_PROPERTY(int totalNumPoints READ getTotalNumPoints NOTIFY totalNumPointsChanged)

public:
  [[nodiscard]] QString getContentHash() const;
  void setContentHash(const QString &contentHash);
  [[nodiscard]] int getNumAchievements() const;
  [[nodiscard]] int getTotalNumPoints() const;

  [[nodiscard]] QString getSetName() const;

signals:
  void contentHashChanged();
  void numAchievementsChanged();
  void totalNumPointsChanged();
  void setNameChanged();

private:
  QString m_contentHash;
  QString m_setName;
  int m_numAchievements = 0;
  int m_totalNumPoints = 0;
  // platform?
  // List of achievements
  // name of set
  // user's progress
};

}
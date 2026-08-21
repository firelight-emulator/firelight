#pragma once
#include "AchievementSetItem.hpp"

#include <firelight/platforms/platform.hpp>

#include <QObject>

namespace firelight::achievements {

class RetroAchievementsGameItem : public QObject, public ServiceAccessor {
  Q_OBJECT
  Q_PROPERTY(QString contentHash READ getContentHash WRITE setContentHash NOTIFY contentHashChanged)
  Q_PROPERTY(QString title MEMBER m_title NOTIFY contentHashChanged)
  Q_PROPERTY(QString iconUrl MEMBER m_iconUrl NOTIFY contentHashChanged)
  Q_PROPERTY(bool hardcore READ isHardcore WRITE setHardcore NOTIFY hardcoreChanged)
  Q_PROPERTY(QString platformName MEMBER m_platformName NOTIFY contentHashChanged)

  Q_PROPERTY(QList<AchievementSetItem *> achievementSets READ getAchievementSets NOTIFY contentHashChanged)

public:
  explicit RetroAchievementsGameItem(QObject *parent = nullptr);
  [[nodiscard]] QString getContentHash() const;
  void setContentHash(const QString &contentHash);

  [[nodiscard]] bool isHardcore() const { return m_hardcore; }

  void setHardcore(const bool hardcore) {
    if (m_hardcore == hardcore) {
      return;
    }
    m_hardcore = hardcore;

    for (const auto &setItem : m_achievementSets) {
      setItem->setHardcore(m_hardcore);
    }

    emit hardcoreChanged();
  }

  [[nodiscard]] QList<AchievementSetItem *> getAchievementSets() const;

signals:
  void contentHashChanged();
  void hardcoreChanged();

private:
  QString m_contentHash;
  QString m_title;
  QString m_iconUrl;
  QString m_platformName;
  bool m_hardcore = false;

  platforms::Platform m_platform;

  QList<AchievementSetItem *> m_achievementSets;
};

} // namespace firelight::achievements

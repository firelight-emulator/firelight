#include "AchievementSetItem.hpp"

#include <QJsonObject>
#include <QtConcurrent>

#include <cpr/cpr.h>
#include <rcheevos/ra_client.hpp>
#include <rcheevos/ra_constants.h>
#include <spdlog/spdlog.h>

namespace firelight::achievements {

QString AchievementSetItem::getContentHash() const { return m_contentHash; }

void AchievementSetItem::setContentHash(const QString &contentHash) {
  if (m_contentHash == contentHash) {
    return;
  }

  auto user = getAchievementManager()->getCurrentUser();
  if (!user.has_value()) {
    spdlog::error("[RetroAchievements] User not logged in");
    return;
  }

  auto gameId = getAchievementManager()->cache().getGameIdFromHash(
      contentHash.toStdString());
  if (!gameId.has_value()) {
    spdlog::error("[RetroAchievements] Failed to get game ID from hash: {}",
                  contentHash.toStdString());
    return;
  }

  // QtConcurrent::run([this, contentHash] {
  const auto url = cpr::Url{
      "https://retroachievements.org/API/API_GetGameInfoAndUserProgress.php"};
  const auto headers = cpr::Header{{"User-Agent", USER_AGENT},
                                   {"Content-Type", "application/json"}};

  cpr::Parameters params = {{"g", std::to_string(gameId.value())},
                            {"z", user->username},
                            {"y", "i14akX8NLePPVzUtry5Pi2POkCxZMPDK"},
                            {"u", user->username}};

  const auto response = Post(url, headers, params);

  if (response.status_code != 200) {
    spdlog::error(
        "[RetroAchievements] Failed to get achievements for game ID: {}",
        gameId.value());
    return;
  }

  auto json = QJsonDocument::fromJson(QByteArray::fromStdString(response.text))
                  .object();
  if (json.isEmpty()) {
    return;
  }

  auto achievements = json["Achievements"].toObject();
  m_setName = json["Title"].toString();
  emit setNameChanged();

  QVector<Achievement> results;
  m_numAchievements = 0;
  for (auto val = achievements.begin(); val != achievements.end(); ++val) {
    auto jsonObj = val.value().toObject();

    Achievement achievement;
    achievement.id = jsonObj["ID"].toInt();
    achievement.name = jsonObj["Title"].toString().toStdString();
    achievement.description = jsonObj["Description"].toString().toStdString();
    achievement.points = jsonObj["Points"].toInt();
    achievement.earnedDate = jsonObj["DateEarned"].toString().toStdString();
    achievement.earnedDateHardcore =
        jsonObj["DateEarnedHardcore"].toString().toStdString();
    achievement.displayOrder = jsonObj["DisplayOrder"].toInt();
    achievement.type = jsonObj["Type"].toString().toStdString();

    QString imageUrl;
    if (jsonObj.contains("DateEarnedHardcore") &&
        !jsonObj["DateEarnedHardcore"].toString().isEmpty()) {
      achievement.earned = true;
      imageUrl = "https://media.retroachievements.org/Badge/" +
                 jsonObj["BadgeName"].toString() + ".png";
    } else {
      achievement.earned = false;
      imageUrl = "https://media.retroachievements.org/Badge/" +
                 jsonObj["BadgeName"].toString() + "_lock.png";
    }

    achievement.imageUrl = imageUrl.toStdString();

    results.append(achievement);
    m_numAchievements++;
  }

  std::ranges::sort(results, [](const Achievement &a, const Achievement &b) {
    return a.displayOrder < b.displayOrder;
  });

  m_achievementListModel.refreshAchievements(results);

  m_contentHash = contentHash;
  emit contentHashChanged();
  emit achievementsChanged();
  // });
}
int AchievementSetItem::getNumAchievements() const { return m_numAchievements; }
int AchievementSetItem::getTotalNumPoints() const { return m_totalNumPoints; }
QString AchievementSetItem::getSetName() const { return m_setName; }

int AchievementSetItem::getSetId() const { return m_setId; }
gui::AchievementListModel *AchievementSetItem::getAchievements() const {
  return const_cast<gui::AchievementListModel *>(&m_achievementListModel);
}
} // namespace firelight::achievements
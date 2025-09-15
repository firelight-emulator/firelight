#include "game_activity_item.hpp"

namespace firelight::activity {

QString GameActivityItem::getContentHash() const { return m_contentHash; }

void GameActivityItem::setContentHash(const QString &contentHash) {
  if (m_contentHash == contentHash) {
    return;
  }

  m_contentHash = contentHash;

  auto playSessions =
      getActivityLog()->getPlaySessions(m_contentHash.toStdString());

  m_playSessions->refreshItems(playSessions);

  m_timesPlayed = 0;
  m_totalSecondsPlayed = 0;
  for (const auto &session : playSessions) {
    printf("Play session: %s, %llu, %llu, %llu\n", session.contentHash.c_str(),
           session.startTime, session.endTime, session.unpausedDurationMillis);
    m_timesPlayed++;
    m_totalSecondsPlayed += session.unpausedDurationMillis / 1000;
  }

  emit timesPlayedChanged();
  emit totalSecondsPlayedChanged();
  emit playSessionsChanged();
  emit contentHashChanged();
}

uint64_t GameActivityItem::getTotalSecondsPlayed() const {
  return m_totalSecondsPlayed;
}

QAbstractListModel *GameActivityItem::getPlaySessions() const {
  return m_playSessions;
}

} // namespace firelight::activity

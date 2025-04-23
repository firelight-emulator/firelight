#pragma once
#include "play_session_list_model.hpp"

#include <QObject>
#include <manager_accessor.hpp>
namespace firelight::activity {

class GameActivityItem : public QObject, public ManagerAccessor {
  Q_OBJECT
  Q_PROPERTY(QString contentHash READ getContentHash WRITE setContentHash NOTIFY
                 contentHashChanged)
  Q_PROPERTY(int timesPlayed READ getTimesPlayed NOTIFY timesPlayedChanged)
  Q_PROPERTY(int totalSecondsPlayed READ getTotalSecondsPlayed NOTIFY
                 totalSecondsPlayedChanged)
  Q_PROPERTY(QAbstractListModel *playSessions READ getPlaySessions NOTIFY
                 playSessionsChanged)

public:
  QString getContentHash() const;
  void setContentHash(const QString &contentHash);

  int getTimesPlayed() const { return m_timesPlayed; }
  int getTotalSecondsPlayed() const;

  QAbstractListModel *getPlaySessions() const;

signals:
  void contentHashChanged();
  void timesPlayedChanged();
  void totalSecondsPlayedChanged();
  void playSessionsChanged();

private:
  QString m_contentHash;
  int m_timesPlayed = 0;
  int m_totalSecondsPlayed = 0;

  PlaySessionListModel *m_playSessions = new PlaySessionListModel();
};

} // namespace firelight::activity
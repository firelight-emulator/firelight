#pragma once

#include "activity/activity_log.hpp"

#include <QSqlDatabase>

namespace firelight::activity {
class SqliteActivityLog final : public IActivityLog {
public:
  explicit SqliteActivityLog(QString dbPath);

  bool createPlaySession(PlaySession &session) override;

  std::optional<PlaySession>
  getLatestPlaySession(std::string contentHash) override;

  [[nodiscard]] QSqlDatabase getDatabase() const;

  std::vector<PlaySession> getPlaySessions(std::string contentHash) override;
  std::vector<PlaySession> getPlaySessions() override;

private:
  QString databasePath;
};
} // namespace firelight::activity
// firelight

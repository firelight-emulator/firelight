#pragma once

#include <firelight/activity/activity_log.hpp>

#include <memory>
#include <mutex>
#include <string>

namespace SQLite {
class Database;
}

namespace firelight::activity {
class SqliteActivityLog final : public IActivityLog {
public:
  explicit SqliteActivityLog(std::string databaseFile);
  ~SqliteActivityLog() override;

  bool createPlaySession(PlaySession &session) override;

  std::optional<PlaySession> getLatestPlaySession(std::string contentHash) override;

  std::vector<PlaySession> getPlaySessions(std::string contentHash) override;
  std::vector<PlaySession> getPlaySessions() override;

private:
  std::string m_databaseFile;
  std::unique_ptr<SQLite::Database> m_db;
  mutable std::mutex m_mutex;
};
} // namespace firelight::activity

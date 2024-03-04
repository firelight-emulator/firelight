#pragma once

#include "userdata_database.hpp"
#include <QSqlDatabase>
#include <filesystem>

class SqliteUserdataDatabase final : public IUserdataDatabase {
public:
  explicit SqliteUserdataDatabase(const std::filesystem::path &dbFile);
  void savePlaySession(std::string romMd5, long long startTime,
                       long long endTime,
                       long long unpausedDurationSeconds) override;

private:
  std::filesystem::path m_database_path;
  QSqlDatabase m_database;
};

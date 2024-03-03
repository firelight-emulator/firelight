//
// Created by alexs on 2/16/2024.
//

#ifndef SQLITE_USERDATA_DATABASE_HPP
#define SQLITE_USERDATA_DATABASE_HPP
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

#endif // SQLITE_USERDATA_DATABASE_HPP

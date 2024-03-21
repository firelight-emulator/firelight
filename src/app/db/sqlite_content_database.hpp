#pragma once

#include "firelight/content_database.hpp"
#include <filesystem>
#include <optional>
#include <sqlite3.h>
#include <string>

class SqliteContentDatabase final : public IContentDatabase {
public:
  explicit SqliteContentDatabase(std::filesystem::path dbFile);
  ~SqliteContentDatabase() override = default;
  std::optional<ROM> getRomByMd5(const std::string &md5) override;
  std::optional<Game> getGameByRomId(int romId) override;
  std::optional<Romhack> getRomhackByMd5(const std::string &md5) override;
  std::optional<Platform> getPlatformByExtension(std::string ext) override;

private:
  sqlite3 *getOrCreateDbConnection();
  ROM romFromDbRow(sqlite3_stmt *stmt);
  Game gameFromDbRow(sqlite3_stmt *stmt);

  std::filesystem::path databaseFile;
  sqlite3 *database = nullptr;
};

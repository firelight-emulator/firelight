#pragma once

#include "firelight/content_database.hpp"

#include <QObject>
#include <QSqlDatabase>
#include <filesystem>
#include <optional>
#include <sqlite3.h>
#include <string>

class SqliteContentDatabase final : public QObject, public IContentDatabase {
  Q_OBJECT
public:
  std::optional<firelight::db::Game> getGame(int id) override;
  explicit SqliteContentDatabase(std::filesystem::path dbFile);
  ~SqliteContentDatabase() override;
  std::optional<ROM> getRomByMd5(const std::string &md5) override;
  std::optional<Game> getGameByRomId(int romId) override;
  std::optional<Romhack> getRomhackByMd5(const std::string &md5) override;
  std::optional<Platform> getPlatformByExtension(std::string ext) override;
  bool tableExists(const std::string &tableName) override;
  std::optional<ROM> getRom(int id) override;
  std::optional<firelight::db::Mod> getMod(int id) override;
  std::optional<firelight::db::Patch> getPatch(int id) override;
  std::optional<Platform> getPlatform(int id) override;
  std::optional<firelight::db::Region> getRegion(int id) override;
  std::vector<firelight::db::Mod> getAllMods() override;
  std::vector<ROM> getMatchingRoms(const ROM &rom) override;
  std::vector<firelight::db::Patch>
  getMatchingPatches(const firelight::db::Patch &patch) override;

private:
  sqlite3 *getOrCreateDbConnection();
  ROM romFromDbRow(sqlite3_stmt *stmt);
  Game gameFromDbRow(sqlite3_stmt *stmt);
  std::filesystem::path databaseFile;
  sqlite3 *database = nullptr;
  [[nodiscard]] QSqlDatabase getDatabase() const;
  firelight::db::Mod createModFromQuery(const QSqlQuery &query);
};

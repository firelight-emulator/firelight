#pragma once

#include "firelight/content_database.hpp"

#include <QObject>
#include <QSqlDatabase>
#include <filesystem>
#include <optional>
#include <string>

namespace firelight::db {
  class SqliteContentDatabase final : public QObject, public IContentDatabase {
    Q_OBJECT

  public:
    explicit SqliteContentDatabase(std::filesystem::path dbFile);

    ~SqliteContentDatabase() override;

    bool tableExists(const std::string &tableName) override;

    std::optional<Game> getGame(int id) override;

    std::optional<ROM> getRom(int id) override;

    std::vector<ROM> getMatchingRoms(const ROM &rom) override;

    std::optional<int> getRetroAchievementsIdForGame(int id) override;

    std::optional<Mod> getMod(int id) override;

    std::vector<Mod> getAllMods() override;

    std::optional<Patch> getPatch(int id) override;

    std::vector<Patch> getMatchingPatches(const Patch &patch) override;

    std::optional<Platform> getPlatform(int id) override;

    std::vector<Platform> getMatchingPlatforms(const Platform &platform) override;

    std::optional<Region> getRegion(int id) override;

  private:
    std::filesystem::path databaseFile;

    [[nodiscard]] QSqlDatabase getDatabase() const;

    static Game createGameFromQuery(const QSqlQuery &query);

    static Mod createModFromQuery(const QSqlQuery &query);

    static ROM createRomFromQuery(const QSqlQuery &query);

    static Patch createPatchFromQuery(const QSqlQuery &query);

    static Platform createPlatformFromQuery(const QSqlQuery &query);

    static Region createRegionFromQuery(const QSqlQuery &query);
  };
} // namespace firelight::db

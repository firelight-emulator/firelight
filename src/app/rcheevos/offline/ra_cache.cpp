#include "ra_cache.hpp"
#include <QSqlError>
#include <QSqlQuery>
#include <QThread>
#include <spdlog/spdlog.h>

namespace firelight::achievements {
    constexpr auto RCHEEVOS_DATABASE_PREFIX = "rcheevos_cache_";

    RetroAchievementsCache::RetroAchievementsCache(QString dbFile)
        : databaseFile(std::move(dbFile)) {
        std::string setupQueryString = R"(
            CREATE TABLE IF NOT EXISTS hashes (
                hash TEXT NOT NULL,
                achievement_set_id INTEGER NOT NULL,
                PRIMARY KEY (hash)
            );

            CREATE TABLE IF NOT EXISTS achievement_sets (
                id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
                name TEXT NOT NULL,
                num_achievements INTEGER NOT NULL
            );

            CREATE TABLE IF NOT EXISTS users (
                username TEXT PRIMARY KEY NOT NULL,
                token TEXT NOT NULL,
                points INTEGER NOT NULL,
                hardcore_points INTEGER NOT NULL
            );

            CREATE TABLE IF NOT EXISTS user_achievement_sets (
                username TEXT NOT NULL,
                achievement_set_id INTEGER NOT NULL,
                num_earned INTEGER NOT NULL,
                num_earned_hardcore INTEGER NOT NULL,
                PRIMARY KEY (username, achievement_set_id),
                FOREIGN KEY (achievement_set_id) REFERENCES achievement_sets(id)
            );

            CREATE TABLE IF NOT EXISTS user_unlocks (
                username TEXT NOT NULL,
                achievement_id INTEGER NOT NULL,
                earned BOOLEAN NOT NULL,
                earned_hardcore BOOLEAN NOT NULL,
                "when" TIMESTAMP,
                when_hardcore TIMESTAMP,
                synced BOOLEAN DEFAULT 0 NOT NULL,
                synced_hardcore BOOLEAN DEFAULT 0 NOT NULL,
                PRIMARY KEY (username, achievement_id),
                FOREIGN KEY (achievement_id) REFERENCES achievements(id)
            );

            CREATE TABLE IF NOT EXISTS achievements (
                id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
                name TEXT NOT NULL,
                description TEXT NOT NULL,
                points INTEGER NOT NULL,
                achievement_set_id INTEGER NOT NULL,
                flags INTEGER NOT NULL,
                FOREIGN KEY (achievement_set_id) REFERENCES achievement_sets(id)
            );

            CREATE TABLE IF NOT EXISTS patch_response_cache (
                game_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
                cached_value BLOB NOT NULL
            );
        )";

        QStringList queries = QString::fromStdString(setupQueryString).split(';', Qt::SkipEmptyParts);

        for (const QString &query: queries) {
            if (query.isEmpty() || query.trimmed().isEmpty()) {
                continue;
            }
            if (QSqlQuery setupQuery(getDatabase()); !setupQuery.exec(query)) {
                spdlog::error("Failed to execute query: {}",
                              setupQuery.lastError().text().toStdString());
            }
        }
    }

    RetroAchievementsCache::~RetroAchievementsCache() {
        for (const auto &name: QSqlDatabase::connectionNames()) {
            if (name.startsWith(RCHEEVOS_DATABASE_PREFIX)) {
                QSqlDatabase::removeDatabase(name);
            }
        }
    }

    std::optional<int> RetroAchievementsCache::getGameIdFromAchievementId(const int achievementId) const {
        QSqlQuery query(getDatabase());
        query.prepare(
            "SELECT achievement_set_id FROM achievements WHERE id == :achievementId");

        query.bindValue(":achievementId", achievementId);

        if (!query.exec()) {
            spdlog::error("Failed to get ID: {}",
                          query.lastError().text().toStdString());
        }

        if (!query.next()) {
            return std::nullopt;
        }

        return query.value(0).toInt();
    }

    std::optional<int> RetroAchievementsCache::getGameIdFromHash(const std::string &hash) const {
        QSqlQuery query(getDatabase());
        query.prepare(
            "SELECT achievement_set_id FROM hashes WHERE hash == :hash");

        query.bindValue(":hash", QString::fromStdString(hash));

        if (!query.exec()) {
            spdlog::error("Failed to update hash: {}",
                          query.lastError().text().toStdString());
            return std::nullopt;
        }

        if (!query.next()) {
            return std::nullopt;
        }

        return query.value(0).toInt();
    }

    int RetroAchievementsCache::getNumRemainingAchievements(const std::string &username, const int gameId,
                                                            const bool hardcore) const {
        QSqlQuery query(getDatabase());
        if (hardcore) {
            query.prepare("WITH thing AS ("
                "SELECT count(*) as unearned, (select count(*) from user_unlocks u JOIN achievements a ON u.achievement_id = a.id WHERE a.achievement_set_id == :gameId AND earned_hardcore == true AND username == :username) as earned "
                "FROM main.achievements WHERE flags == 3 AND achievement_set_id == :gameId) "
                "SELECT (unearned - earned) as remaining FROM thing");
        } else {
            query.prepare("WITH thing AS ("
                "SELECT count(*) as unearned, (select count(*) from user_unlocks u JOIN achievements a ON u.achievement_id = a.id WHERE a.achievement_set_id == :gameId AND earned == true AND username == :username) as earned "
                "FROM main.achievements WHERE flags == 3 AND achievement_set_id == :gameId) "
                "SELECT (unearned - earned) as remaining FROM thing");
        }

        query.bindValue(":username", QString::fromStdString(username));
        query.bindValue(":gameId", gameId);

        if (!query.exec()) {
            spdlog::error("Failed to get number of remaining achievements: {}",
                          query.lastError().text().toStdString());
            return -1;
        }

        if (!query.next()) {
            spdlog::error("Failed to get number of remaining achievements: {}",
                          query.lastError().text().toStdString());
            return -1;
        }

        return query.value(0).toInt();
        // QSqlQuery query(getDatabase());
        // query.prepare(
        //     "SELECT remaining_achievements FROM remaining_achievements_view WHERE username == :username AND game_id == :gameId");
        //
        //
        // query.bindValue(":username", QString::fromStdString(username));
        // query.bindValue(":gameId", gameId);
        //
        // if (!query.exec()) {
        //     spdlog::error("Failed to create user: {}",
        //                   query.lastError().text().toStdString());
        //
        //     return -1;
        // }
        //
        // if (!query.next()) {
        //     return -1;
        // }
        //
        // return query.value(0).toInt();
    }

    bool RetroAchievementsCache::awardAchievement(const std::string &username, const std::string &token,
                                                  const int achievementId, const bool hardcore) const {
        QSqlQuery checkQuery(getDatabase());
        if (hardcore) {
            checkQuery.prepare(
                "SELECT earned_hardcore FROM user_unlocks WHERE username == :username AND achievement_id == :achievementId");
        } else {
            checkQuery.prepare(
                "SELECT earned FROM user_unlocks WHERE username == :username AND achievement_id == :achievementId");
        }
        checkQuery.bindValue(":username", QString::fromStdString(username));
        checkQuery.bindValue(":achievementId", achievementId);

        if (!checkQuery.exec()) {
            spdlog::error("Failed to check earned status: {}",
                          checkQuery.lastError().text().toStdString());
            return false;
        }

        if (!checkQuery.next()) {
            spdlog::error("Failed to get result: {}", checkQuery.lastError().text().toStdString());
        }

        if (checkQuery.value(0).toBool()) {
            return true;
        }

        const auto duration = std::chrono::system_clock::now().time_since_epoch();
        const auto epochSeconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();

        QSqlQuery query(getDatabase());
        if (hardcore) {
            query.prepare(
                "UPDATE user_unlocks SET earned_hardcore = 1, when_hardcore = :timestamp, synced_hardcore = 0 WHERE username == :username AND achievement_id == :achievementId");
        } else {
            query.prepare(
                "UPDATE user_unlocks SET earned = 1, when = :timestamp, synced = 0 WHERE username == :username AND achievement_id == :achievementId");
        }

        query.bindValue(":timestamp", epochSeconds);
        query.bindValue(":username", QString::fromStdString(username));
        query.bindValue(":achievementId", achievementId);

        if (!query.exec()) {
            spdlog::error("Failed to mark achievement unlocked: {}",
                          query.lastError().text().toStdString());
            return false;
        }

        // Get achievement points
        QSqlQuery pointsQuery(getDatabase());
        pointsQuery.prepare(
            "SELECT points FROM achievements WHERE id == :achievementId");
        pointsQuery.bindValue(":achievementId", achievementId);

        if (!pointsQuery.exec()) {
            spdlog::error("Failed to get points: {}",
                          pointsQuery.lastError().text().toStdString());
            return false;
        }

        if (!pointsQuery.next()) {
            return false;
        }

        const auto points = pointsQuery.value(0).toInt();

        const auto current = getUserScore(username, hardcore);
        if (current == -1) {
            return false;
        }

        setUserScore(username, current + points, hardcore);

        return true;
    }

    bool RetroAchievementsCache::markAchievementUnlocked(
        const std::string &username, const int achievementId,
        const bool hardcore, const long long epochSeconds) const {
      QSqlQuery query(getDatabase());
      if (hardcore) {
        query.prepare(
            "UPDATE user_unlocks SET earned_hardcore = 1, when_hardcore = "
            ":timestamp, synced_hardcore = 1 WHERE username == :username AND "
            "achievement_id == :achievementId");
      } else {
        query.prepare("UPDATE user_unlocks SET earned = 1, \"when\" = "
                      ":timestamp, synced = 1 WHERE username == :username AND "
                      "achievement_id == :achievementId");
      }

      query.bindValue(":timestamp", epochSeconds);
      query.bindValue(":username", QString::fromStdString(username));
      query.bindValue(":achievementId", achievementId);

      if (!query.exec()) {
        spdlog::error("Failed to mark achievement unlocked: {}",
                      query.lastError().text().toStdString());
        return false;
      }

      return true;
    }
    bool RetroAchievementsCache::markAchievementLocked(
        const std::string &username, const int achievementId, const bool hardcore) const {
      QSqlQuery query(getDatabase());
      if (hardcore) {
        query.prepare(
            "UPDATE user_unlocks SET earned_hardcore = 0, when_hardcore = NULL, "
            "synced_hardcore = 1 WHERE username == :username AND "
            "achievement_id == :achievementId");
      } else {
        query.prepare("UPDATE user_unlocks SET earned = 0, \"when\" = NULL, "
                      "synced = 1 WHERE username == :username AND "
                      "achievement_id == :achievementId");
      }

      query.bindValue(":username", QString::fromStdString(username));
      query.bindValue(":achievementId", achievementId);

      if (!query.exec()) {
        spdlog::error("Failed to mark achievement locked: {}",
                      query.lastError().text().toStdString());
        return false;
      }

      return true;
    }

    std::vector<CachedAchievement>
    RetroAchievementsCache::getUserAchievements(const std::string &username,
                                                const int gameId) const {
      QSqlQuery query(getDatabase());
      query.prepare(
          "SELECT u.achievement_id,a.achievement_set_id,coalesce(u.\"when\", "
          "0),coalesce(u.when_hardcore, 0),a.points, u.earned, "
          "u.earned_hardcore FROM user_unlocks u "
          "JOIN achievements a ON u.achievement_id = a.id "
          "WHERE u.username == :username AND a.achievement_set_id == :gameId");

      query.bindValue(":username", QString::fromStdString(username));
      query.bindValue(":gameId", gameId);

      if (!query.exec()) {
        spdlog::error("Failed to get unlocks: {}",
                      query.lastError().text().toStdString());
        return {};
      }

      std::vector<CachedAchievement> achievements;
      while (query.next()) {
        achievements.emplace_back(
            CachedAchievement{.ID = query.value(0).toInt(),
                              .GameID = query.value(1).toInt(),
                              .When = query.value(2).toInt(),
                              .WhenHardcore = query.value(3).toInt(),
                              .Points = query.value(4).toInt(),
                              .Earned = query.value(5).toBool(),
                              .EarnedHardcore = query.value(6).toBool()});
      }

      return achievements;
    }
    std::vector<CachedAchievement>
    RetroAchievementsCache::getUnsyncedAchievements(
        const std::string &username) const {
      QSqlQuery query(getDatabase());
      query.prepare(
          "SELECT u.achievement_id,a.achievement_set_id,coalesce(u.\"when\", "
          "0),coalesce(u.when_hardcore, 0),a.points, u.earned, "
          "u.earned_hardcore, u.synced, u.synced_hardcore FROM user_unlocks u "
          "JOIN achievements a ON u.achievement_id = a.id "
          "WHERE u.username == :username AND ((u.earned == 1 AND u.synced == "
          "0) OR (u.earned_hardcore == 1 AND u.synced_hardcore == 0))");

      query.bindValue(":username", QString::fromStdString(username));

      if (!query.exec()) {
        spdlog::error("Failed to get unlocks: {}",
                      query.lastError().text().toStdString());
        return {};
      }

      std::vector<CachedAchievement> achievements;
      while (query.next()) {
        achievements.emplace_back(
            CachedAchievement{.ID = query.value(0).toInt(),
                              .GameID = query.value(1).toInt(),
                              .When = query.value(2).toLongLong(),
                              .WhenHardcore = query.value(3).toLongLong(),
                              .Points = query.value(4).toInt(),
                              .Earned = query.value(5).toBool(),
                              .EarnedHardcore = query.value(6).toBool(),
                              .Synced = query.value(7).toBool(),
                              .SyncedHardcore = query.value(8).toBool()});
      }

      return achievements;
    }
    std::optional<std::string> RetroAchievementsCache::getHashFromGameId(int gameId) const {
      QSqlQuery query(getDatabase());
      query.prepare(
          "SELECT hash FROM hashes WHERE achievement_set_id == :achievementSetId");

      query.bindValue(":achievementSetId", gameId);

      if (!query.exec()) {
        spdlog::error("Failed to get hash ID: {}",
                      query.lastError().text().toStdString());
        return std::nullopt;
      }

      if (!query.next()) {
        return std::nullopt;
      }

      return {query.value(0).toString().toStdString()};
    }

    bool RetroAchievementsCache::addUser(const std::string &username, const std::string &token) const {
      QSqlQuery query(getDatabase());
        query.prepare(
            "INSERT OR IGNORE INTO users (username, token, hardcore_points, points) VALUES (:username, :token, 0, 0)");

      query.bindValue(":token", QString::fromStdString(token));
      query.bindValue(":username", QString::fromStdString(username));

      if (!query.exec()) {
        spdlog::error("Failed to create user: {}",
                      query.lastError().text().toStdString());
        return false;
      }

      if (query.numRowsAffected() == 0) {
        QSqlQuery updateQuery(getDatabase());
        updateQuery.prepare("UPDATE users SET token = :token WHERE username = :username");

        updateQuery.bindValue(":token", QString::fromStdString(token));
        updateQuery.bindValue(":username", QString::fromStdString(username));

        if (!updateQuery.exec()) {
          spdlog::error("Failed to update user token: {}",
                        updateQuery.lastError().text().toStdString());
          return false;
        }
      }

      return true;
    }

    int RetroAchievementsCache::getUserScore(const std::string &username, const bool hardcore) const {
        QSqlQuery query(getDatabase());
        if (hardcore) {
            query.prepare(
                "SELECT hardcore_points FROM users WHERE username == :username");
        } else {
            query.prepare(
                "SELECT points FROM users WHERE username == :username");
        }

        query.bindValue(":username", QString::fromStdString(username));

        if (!query.exec()) {
            spdlog::error("Failed to get points: {}",
                          query.lastError().text().toStdString());

            query.finish();
        }

        if (!query.next()) {
            return -1;
        }

        return query.value(0).toInt();
    }

    // int DumbAchievementCache::awardAchievement(const int id, long long epochMillis, bool hardcore) {
    //     if (!m_cachedAchievements.contains(id)) {
    //         return -1;
    //     }
    //
    //     auto score = 0;
    //
    //     auto achievement = m_cachedAchievements[id];
    //     if (hardcore) {
    //         achievement.EarnedHardcore = true;
    //         achievement.WhenHardcore = epochMillis;
    //
    //         score = m_lastKnownScore;
    //     } else {
    //         achievement.Earned = true;
    //         achievement.When = epochMillis;
    //
    //         score = m_lastKnownSoftcoreScore;
    //     }
    //
    //     score += achievement.Points;
    //     return score;
    // }

    std::optional<PatchResponse> RetroAchievementsCache::getPatchResponse(const int gameId) const {
        QSqlQuery query(getDatabase());
        query.prepare(
            "SELECT cached_value FROM patch_response_cache WHERE game_id == :gameId");

        query.bindValue(":gameId", gameId);

        if (!query.exec()) {
            spdlog::error("Failed to get patch response: {}",
                          query.lastError().text().toStdString());
        }

        if (!query.next()) {
            return std::nullopt;
        }

        auto val = query.value(0).toString().toStdString();

        return {nlohmann::json::parse(val).get<PatchResponse>()};
    }

    // void DumbAchievementCache::setStartSessionResponse(const int gameId, const StartSessionResponse &response) {
    //     m_startSessionResponses[gameId] = response;
    //     for (const auto a&: response.Unlocks) {
    //         if (m_cachedAchievements.contains(a.ID)) {
    //             m_cachedAchievements[a.ID].Earned = true;
    //             m_cachedAchievements[a.ID].When = a.When;
    //         }
    //     }
    //
    //     for (const auto a&: response.HardcoreUnlocks) {
    //         if (m_cachedAchievements.contains(a.ID)) {
    //             m_cachedAchievements[a.ID].EarnedHardcore = true;
    //             m_cachedAchievements[a.ID].WhenHardcore = a.When;
    //         }
    //     }
    // }

    void RetroAchievementsCache::setPatchResponse(const std::string &username, const int gameId,
                                                  const PatchResponse &patch) const {
        const auto json = nlohmann::json(patch).dump();

        QSqlQuery patchQuery(getDatabase());
        patchQuery.prepare(
            "INSERT OR IGNORE INTO patch_response_cache (game_id, cached_value) VALUES (:gameId, :cachedValue)");
        patchQuery.bindValue(":gameId", gameId);
        patchQuery.bindValue(":cachedValue", QString::fromStdString(json));

        if (!patchQuery.exec()) {
            spdlog::error("Failed to add patch response: {}",
                          patchQuery.lastError().text().toStdString());
        }

        // TODO: Only insert ones that have the flags set right
        // TODO: Remove ones that aren't present in the patch response
        for (const auto &a: patch.PatchData.Achievements) {
            QSqlQuery query(getDatabase());
            query.prepare(
                "INSERT OR IGNORE INTO achievements (id, name, description, achievement_set_id, points, flags) VALUES (:id, :name, :description, :gameId, :points, :flags)");

            query.bindValue(":id", a.ID);
            query.bindValue(":name", QString::fromStdString(a.Title));
            query.bindValue(":description", QString::fromStdString(a.Description));
            query.bindValue(":gameId", gameId);
            query.bindValue(":points", a.Points);
            query.bindValue(":flags", a.Flags);

            if (!query.exec()) {
                spdlog::error("Failed to add achievement: {}",
                              query.lastError().text().toStdString());
            }

            query.finish();

            QSqlQuery unlockQuery(getDatabase());
            unlockQuery.prepare(
                "INSERT OR IGNORE INTO user_unlocks (username, achievement_id, earned, earned_hardcore) VALUES (:username, :achievementId, 0, 0)");
            unlockQuery.bindValue(":username", QString::fromStdString(username));
            unlockQuery.bindValue(":achievementId", a.ID);

            if (!unlockQuery.exec()) {
                spdlog::error("Failed to add user unlock: {}",
                              unlockQuery.lastError().text().toStdString());
            }
        }
    }

    void RetroAchievementsCache::setGameId(const std::string &hash, const int id) const {
        QSqlQuery query(getDatabase());
        query.prepare(
            "INSERT OR IGNORE INTO hashes (hash, achievement_set_id) VALUES (:hash, :gameId)");


        query.bindValue(":hash", QString::fromStdString(hash));
        query.bindValue(":gameId", id);

        if (!query.exec()) {
            spdlog::error("Failed to update hash: {}",
                          query.lastError().text().toStdString());
        }
    }

    void RetroAchievementsCache::setUserScore(const std::string &username, const int score, const bool hardcore) const {
        // Create row if it doesnt exist
        QSqlQuery query(getDatabase());
        if (hardcore) {
            query.prepare(
                "INSERT OR IGNORE INTO users (username, hardcore_points, points) VALUES (:username, :score, 0)");
        } else {
            query.prepare(
                "INSERT OR IGNORE INTO users (username, hardcore_points, points) VALUES (:username, 0, :score)");
        }

        query.bindValue(":score", score);
        query.bindValue(":username", QString::fromStdString(username));

        if (!query.exec()) {
            spdlog::error("Failed to create user: {}",
                          query.lastError().text().toStdString());
            return;
        }

        if (query.numRowsAffected() == 0) {
            QSqlQuery updateQuery(getDatabase());
            if (hardcore) {
                updateQuery.prepare("UPDATE users SET hardcore_points = :score WHERE username = :username");
            } else {
                updateQuery.prepare("UPDATE users SET points = :score WHERE username = :username");
            }

            updateQuery.bindValue(":score", score);
            updateQuery.bindValue(":username", QString::fromStdString(username));

            if (!updateQuery.exec()) {
                spdlog::error("Failed to update user points: {}",
                              updateQuery.lastError().text().toStdString());
            }
        }
    }

    QSqlDatabase RetroAchievementsCache::getDatabase() const {
        const auto name =
                RCHEEVOS_DATABASE_PREFIX +
                QString::number(reinterpret_cast<quint64>(QThread::currentThread()), 16);
        if (QSqlDatabase::contains(name)) {
            return QSqlDatabase::database(name);
        }

        spdlog::debug("Database connection with name {} does not exist; creating",
                      name.toStdString());
        auto db = QSqlDatabase::addDatabase("QSQLITE", name);
        db.setDatabaseName(databaseFile);
        db.open();
        return db;
    }
} // namespace firelight::achievements

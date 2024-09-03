#include "ra_cache.hpp"
#include <QSqlError>
#include <QSqlQuery>
#include <QThread>
#include <spdlog/spdlog.h>

namespace firelight::achievements {
    constexpr auto DATABASE_PREFIX = "rcheevos_cache_";

    RetroAchievementsCache::RetroAchievementsCache(std::filesystem::path dbFile)
        : databaseFile(std::move(dbFile)) {
    }

    RetroAchievementsCache::~RetroAchievementsCache() {
        for (const auto &name: QSqlDatabase::connectionNames()) {
            if (name.startsWith(DATABASE_PREFIX)) {
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

        auto val = query.value(0).toInt();

        query.finish();

        return val;
    }

    std::optional<int> RetroAchievementsCache::getGameIdFromHash(const std::string &hash) const {
        QSqlQuery query(getDatabase());
        query.prepare(
            "SELECT achievement_set_id FROM hashes WHERE hash == :hash");

        query.bindValue(":hash", QString::fromStdString(hash));

        if (!query.exec()) {
            spdlog::error("Failed to update hash: {}",
                          query.lastError().text().toStdString());
        }

        if (!query.next()) {
            return std::nullopt;
        }

        auto val = query.value(0).toInt();

        query.finish();

        return val;
    }

    int RetroAchievementsCache::getNumRemainingAchievements(const std::string &username, const int gameId) const {
        QSqlQuery query(getDatabase());
        query.prepare(
            "SELECT remaining_achievements FROM remaining_achievements_view WHERE username == :username AND game_id == :gameId");


        query.bindValue(":username", QString::fromStdString(username));
        query.bindValue(":gameId", gameId);

        if (!query.exec()) {
            spdlog::error("Failed to create user: {}",
                          query.lastError().text().toStdString());

            query.finish();
        }

        if (!query.next()) {
            return -1;
        }

        const auto val = query.value(0).toInt();

        query.finish();

        return val;
    }

    bool RetroAchievementsCache::awardAchievement(const std::string &username, const std::string &token,
                                                  int achievementId, bool hardcore) {
        return false;
    }

    bool RetroAchievementsCache::
    markAchievementUnlocked(const std::string &username, int achievementId, bool hardcore) {
        return false;
    }

    std::vector<CachedAchievement>
    RetroAchievementsCache::getUserAchievements(const std::string &username, int gameId) {
        /* "SELECT * FROM
            user_achievements_view
        WHERE
            username = 'desired_username'
            AND achievement_set_id = desired_achievement_set_id;
        "*/
        return {};
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
            spdlog::error("Failed to create user: {}",
                          query.lastError().text().toStdString());

            query.finish();
        }

        if (!query.next()) {
            return -1;
        }

        const auto val = query.value(0).toInt();

        query.finish();

        return val;
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

    std::optional<PatchResponse> RetroAchievementsCache::getPatchResponse(const int gameId) {
        if (m_patchResponses.contains(gameId)) {
            return m_patchResponses[gameId];
        }

        return std::nullopt;
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

    void RetroAchievementsCache::setPatchResponse(const int gameId, const PatchResponse &patch) {
        // m_patchResponses[gameId] = patch;
        // for (const auto a&: patch.PatchData.Achievements) {
        //     if (!m_cachedAchievements.contains(a.ID)) {
        //         m_cachedAchievements[a.ID] = CachedAchievement{
        //             .ID = a.ID,
        //             .Points = a.Points,
        //             .GameID = gameId,
        //             .Earned = false,
        //             .EarnedHardcore = false
        //         };
        //     }
        // }
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

        query.finish();
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

            query.finish();
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

            updateQuery.finish();
        }

        query.finish();
    }

    QSqlDatabase RetroAchievementsCache::getDatabase() const {
        const auto name =
                DATABASE_PREFIX +
                QString::number(reinterpret_cast<quint64>(QThread::currentThread()), 16);
        if (QSqlDatabase::contains(name)) {
            return QSqlDatabase::database(name);
        }

        spdlog::debug("Database connection with name {} does not exist; creating",
                      name.toStdString());
        auto db = QSqlDatabase::addDatabase("QSQLITE", name);
        db.setDatabaseName(QString::fromStdString(databaseFile.string()));
        db.open();
        return db;
    }
} // namespace firelight::achievements

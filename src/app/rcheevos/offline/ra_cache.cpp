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

  QStringList queries =
      QString::fromStdString(setupQueryString).split(';', Qt::SkipEmptyParts);

  for (const QString &query : queries) {
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
  for (const auto &name : QSqlDatabase::connectionNames()) {
    if (name.startsWith(RCHEEVOS_DATABASE_PREFIX)) {
      QSqlDatabase::removeDatabase(name);
    }
  }
}

bool RetroAchievementsCache::tableExists(const std::string &tableName) const {
  QSqlQuery query(getDatabase());
  query.prepare("SELECT 1 FROM " + QString::fromStdString(tableName) +
                " LIMIT 1;");

  return query.exec();
}

/**********************************************************************/

void RetroAchievementsCache::updateUserAchievementStatus(
    const std::string &username, const int achievementId,
    const UserAchievementStatus &status) const {
  QSqlQuery query(getDatabase());
  query.prepare("INSERT INTO user_unlocks (username, achievement_id, earned, "
                "earned_hardcore, \"when\", when_hardcore) "
                "VALUES (:username, :achievementId, :earned, :earnedHardcore, "
                ":when, :whenHardcore) "
                "ON CONFLICT(username, achievement_id) DO UPDATE SET "
                "earned = excluded.earned, "
                "earned_hardcore = excluded.earned_hardcore, "
                "\"when\" = excluded.\"when\", "
                "when_hardcore = excluded.when_hardcore");

  query.bindValue(":earned", status.achieved);
  query.bindValue(":earnedHardcore", status.achievedHardcore);
  query.bindValue(":when", QVariant::fromValue(status.timestamp));
  query.bindValue(":whenHardcore",
                  QVariant::fromValue(status.timestampHardcore));
  query.bindValue(":username", QString::fromStdString(username));
  query.bindValue(":achievementId", achievementId);

  if (!query.exec()) {
    spdlog::error("Failed to update user achievement status: {}",
                  query.lastError().text().toStdString());
  }
}

std::optional<UserAchievementStatus>
RetroAchievementsCache::getUserAchievementStatus(
    const std::string &username, const int achievementId) const {
  QSqlQuery query(getDatabase());
  query.prepare("SELECT earned, earned_hardcore, \"when\", when_hardcore FROM "
                "user_unlocks WHERE username == :username AND achievement_id "
                "== :achievementId");

  query.bindValue(":username", QString::fromStdString(username));
  query.bindValue(":achievementId", achievementId);

  if (!query.exec()) {
    spdlog::error("Failed to get user achievement status: {}",
                  query.lastError().text().toStdString());
    return std::nullopt;
  }

  if (!query.next()) {
    return std::nullopt;
  }

  auto status = UserAchievementStatus{};
  status.achievementId = achievementId;
  status.achieved = query.value("earned").toBool();
  status.achievedHardcore = query.value("earned_hardcore").toBool();
  status.timestamp = query.value("when").toLongLong();
  status.timestampHardcore = query.value("when_hardcore").toLongLong();

  return {status};
}

int RetroAchievementsCache::createAchievement(
    const Achievement &achievement) const {
  QSqlQuery query(getDatabase());
  query.prepare("INSERT INTO achievements (id, name, description, points, "
                "achievement_set_id, flags) "
                "VALUES (:id, :name, :description, :points, "
                ":achievementSetId, :flags) "
                "ON CONFLICT(id) DO UPDATE SET name = excluded.name, "
                "description = excluded.description, "
                "points = excluded.points, achievement_set_id = "
                "excluded.achievement_set_id, flags = excluded.flags");

  query.bindValue(":id", achievement.id);
  query.bindValue(":name", QString::fromStdString(achievement.name));
  query.bindValue(":description",
                  QString::fromStdString(achievement.description));
  query.bindValue(":points", achievement.points);
  query.bindValue(":achievementSetId", achievement.setId);
  query.bindValue(":flags", achievement.flags);

  if (!query.exec()) {
    spdlog::error("Failed to create achievement: {}",
                  query.lastError().text().toStdString());

    return -1;
  }

  if (query.numRowsAffected() == 0) {
    return achievement.id;
  }

  return query.lastInsertId().toInt();
}

std::optional<Achievement>
RetroAchievementsCache::getAchievement(const int achievementId) const {
  QSqlQuery query(getDatabase());
  query.prepare("SELECT * FROM achievements WHERE id == :achievementId");

  query.bindValue(":achievementId", achievementId);

  if (!query.exec()) {
    spdlog::error("Failed to get achievement: {}",
                  query.lastError().text().toStdString());
    return std::nullopt;
  }

  if (!query.next()) {
    return std::nullopt;
  }

  auto achievement = Achievement{};
  achievement.id = query.value("id").toInt();
  achievement.name = query.value("name").toString().toStdString();
  achievement.description = query.value("description").toString().toStdString();
  achievement.points = query.value("points").toInt();
  achievement.setId = query.value("achievement_set_id").toInt();
  achievement.flags = query.value("flags").toInt();

  return {achievement};
}

std::optional<User>
RetroAchievementsCache::getUser(const std::string &username) const {
  QSqlQuery query(getDatabase());
  query.prepare("SELECT * FROM users WHERE username == :username");

  query.bindValue(":username", QString::fromStdString(username));

  if (!query.exec()) {
    spdlog::error("Failed to get user: {}",
                  query.lastError().text().toStdString());
    return std::nullopt;
  }

  if (!query.next()) {
    return std::nullopt;
  }

  User user{};
  user.username = query.value("username").toString().toStdString();
  user.token = query.value("token").toString().toStdString();
  user.points = query.value("points").toInt();
  user.hardcore_points = query.value("hardcore_points").toInt();

  return {user};
}
void RetroAchievementsCache::createUser(const std::string &username,
                                        const std::string &token, int points,
                                        int hardcorePoints) const {
  QSqlQuery query(getDatabase());
  query.prepare("INSERT INTO users (username, token, points, hardcore_points) "
                "VALUES (:username, :token, :points, :hardcorePoints)");

  query.bindValue(":username", QString::fromStdString(username));
  query.bindValue(":token", QString::fromStdString(token));
  query.bindValue(":points", points);
  query.bindValue(":hardcorePoints", hardcorePoints);

  if (!query.exec()) {
    spdlog::error("Failed to create user: {}",
                  query.lastError().text().toStdString());
  }
}

void RetroAchievementsCache::createGameHashRecord(const std::string &hash,
                                                  const int gameId) const {
  QSqlQuery query(getDatabase());
  query.prepare("INSERT OR IGNORE INTO hashes (hash, achievement_set_id) "
                "VALUES (:hash, :gameId)");

  query.bindValue(":hash", QString::fromStdString(hash));
  query.bindValue(":gameId", gameId);

  if (!query.exec()) {
    spdlog::error("Failed to create game hash record: {}",
                  query.lastError().text().toStdString());
  }
}

/**********************************************************************/

std::optional<int> RetroAchievementsCache::getGameIdFromAchievementId(
    const int achievementId) const {
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

std::optional<int>
RetroAchievementsCache::getGameIdFromHash(const std::string &hash) const {
  QSqlQuery query(getDatabase());
  query.prepare("SELECT achievement_set_id FROM hashes WHERE hash == :hash");

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

int RetroAchievementsCache::getNumRemainingAchievements(
    const std::string &username, const int gameId, const bool hardcore) const {
  QSqlQuery query(getDatabase());
  if (hardcore) {
    query.prepare(
        "WITH thing AS ("
        "SELECT count(*) as unearned, (select count(*) from user_unlocks u "
        "JOIN achievements a ON u.achievement_id = a.id WHERE "
        "a.achievement_set_id == :gameId AND earned_hardcore == true AND "
        "username == :username) as earned "
        "FROM main.achievements WHERE flags == 3 AND achievement_set_id == "
        ":gameId) "
        "SELECT (unearned - earned) as remaining FROM thing");
  } else {
    query.prepare("WITH thing AS ("
                  "SELECT count(*) as unearned, (select count(*) from "
                  "user_unlocks u JOIN achievements a ON u.achievement_id = "
                  "a.id WHERE a.achievement_set_id == :gameId AND earned == "
                  "true AND username == :username) as earned "
                  "FROM main.achievements WHERE flags == 3 AND "
                  "achievement_set_id == :gameId) "
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
}

bool RetroAchievementsCache::markAchievementUnlocked(
    const std::string &username, const int achievementId, const bool hardcore,
    const long long epochSeconds) const {
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
bool RetroAchievementsCache::markAchievementLocked(const std::string &username,
                                                   const int achievementId,
                                                   const bool hardcore) const {
  QSqlQuery query(getDatabase());
  if (hardcore) {
    query.prepare("UPDATE user_unlocks SET earned_hardcore = 0, "
                  "when_hardcore = NULL, "
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
bool RetroAchievementsCache::markAchievementSynced(const std::string &username,
                                                   const int achievementId,
                                                   bool hardcore) {
  QSqlQuery query(getDatabase());
  if (hardcore) {
    query.prepare("UPDATE user_unlocks SET synced_hardcore = 1 WHERE username "
                  "== :username AND achievement_id == :achievementId");
  } else {
    query.prepare("UPDATE user_unlocks SET synced = 1 WHERE username == "
                  ":username AND achievement_id == :achievementId");
  }

  query.bindValue(":username", QString::fromStdString(username));
  query.bindValue(":achievementId", achievementId);

  if (!query.exec()) {
    spdlog::error("Failed to mark achievement synced: {}",
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
std::vector<CachedAchievement> RetroAchievementsCache::getUnsyncedAchievements(
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
std::optional<std::string>
RetroAchievementsCache::getHashFromGameId(int gameId) const {
  QSqlQuery query(getDatabase());
  query.prepare("SELECT hash FROM hashes WHERE achievement_set_id == "
                ":achievementSetId");

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
std::vector<User> RetroAchievementsCache::getUsers() const {
  QSqlQuery query(getDatabase());
  query.prepare("SELECT * FROM users");

  if (!query.exec()) {
    spdlog::error("Failed to get users: {}",
                  query.lastError().text().toStdString());
    return {};
  }

  std::vector<User> users;
  while (query.next()) {
    users.emplace_back(User{
        .username = query.value(0).toString().toStdString(),
        .token = query.value(1).toString().toStdString(),
        .points = query.value(2).toInt(),
        .hardcore_points = query.value(3).toInt(),
    });
  }

  return users;
}

bool RetroAchievementsCache::addUser(const std::string &username,
                                     const std::string &token) const {
  QSqlQuery query(getDatabase());
  query.prepare("INSERT OR IGNORE INTO users (username, token, "
                "hardcore_points, points) VALUES (:username, :token, 0, 0)");

  query.bindValue(":token", QString::fromStdString(token));
  query.bindValue(":username", QString::fromStdString(username));

  if (!query.exec()) {
    spdlog::error("Failed to create user: {}",
                  query.lastError().text().toStdString());
    return false;
  }

  if (query.numRowsAffected() == 0) {
    QSqlQuery updateQuery(getDatabase());
    updateQuery.prepare(
        "UPDATE users SET token = :token WHERE username = :username");

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

int RetroAchievementsCache::getUserScore(const std::string &username,
                                         const bool hardcore) const {
  QSqlQuery query(getDatabase());
  if (hardcore) {
    query.prepare(
        "SELECT hardcore_points FROM users WHERE username == :username");
  } else {
    query.prepare("SELECT points FROM users WHERE username == :username");
  }

  query.bindValue(":username", QString::fromStdString(username));

  if (!query.exec()) {
    spdlog::error("Failed to get points: {}",
                  query.lastError().text().toStdString());

    query.finish();
  }

  if (!query.next()) {
    return 0;
  }

  const auto result = query.value(0).toInt();
  return result;
}

std::optional<PatchResponse>
RetroAchievementsCache::getPatchResponse(const int gameId) const {
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

void RetroAchievementsCache::setPatchResponse(
    const std::string &username, const int gameId,
    const PatchResponse &patch) const {
  const auto json = nlohmann::json(patch).dump();

  QSqlQuery patchQuery(getDatabase());
  patchQuery.prepare("INSERT OR IGNORE INTO patch_response_cache (game_id, "
                     "cached_value) VALUES (:gameId, :cachedValue)");
  patchQuery.bindValue(":gameId", gameId);
  patchQuery.bindValue(":cachedValue", QString::fromStdString(json));

  if (!patchQuery.exec()) {
    spdlog::error("Failed to add patch response: {}",
                  patchQuery.lastError().text().toStdString());
  }
}

void RetroAchievementsCache::setGameId(const std::string &hash,
                                       const int id) const {
  QSqlQuery query(getDatabase());
  query.prepare("INSERT OR IGNORE INTO hashes (hash, achievement_set_id) "
                "VALUES (:hash, :gameId)");

  query.bindValue(":hash", QString::fromStdString(hash));
  query.bindValue(":gameId", id);

  if (!query.exec()) {
    spdlog::error("Failed to update hash: {}",
                  query.lastError().text().toStdString());
  }
}

void RetroAchievementsCache::setUserScore(const std::string &username,
                                          const int score,
                                          const bool hardcore) const {
  QSqlQuery query(getDatabase());
  if (hardcore) {
    query.prepare(
        "INSERT INTO users (username, token, hardcore_points, points) "
        "VALUES (:username, \"\", :score, 0) "
        "ON CONFLICT(username) DO UPDATE SET hardcore_points = :score");
  } else {
    query.prepare(
        "INSERT INTO users (username, token, hardcore_points, points) "
        "VALUES (:username, \"\", 0, :score) "
        "ON CONFLICT(username) DO UPDATE SET points = :score");
  }

  query.bindValue(":score", score);
  query.bindValue(":username", QString::fromStdString(username));

  if (!query.exec()) {
    spdlog::error("Failed to set user score: {}",
                  query.lastError().text().toStdString());
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

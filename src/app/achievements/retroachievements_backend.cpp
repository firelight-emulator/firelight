#include "retroachievements_backend.hpp"

#include <QThread>
#include <spdlog/spdlog.h>

namespace firelight::achievements {

constexpr auto RCHEEVOS_DATABASE_PREFIX = "rcheevos_";

RetroAchievementsBackend::RetroAchievementsBackend() {}

void RetroAchievementsBackend::updateUserScore(const std::string &username,
                                               int newScore, bool hardcore) {}

void RetroAchievementsBackend::incrementUserScore(const std::string &username,
                                                  int pointsToAdd,
                                                  bool hardcore) {}

void RetroAchievementsBackend::updateAchievementStatus(
    const std::string &username, UserAchievementStatus status) {}

QSqlDatabase RetroAchievementsBackend::getDatabase() const {
  const auto name =
      RCHEEVOS_DATABASE_PREFIX +
      QString::number(reinterpret_cast<quint64>(QThread::currentThread()), 16);
  if (QSqlDatabase::contains(name)) {
    return QSqlDatabase::database(name);
  }

  spdlog::debug("Database connection with name {} does not exist; creating",
                name.toStdString());
  auto db = QSqlDatabase::addDatabase("QSQLITE", name);
  db.setDatabaseName(m_dbFilename);
  db.open();
  return db;
}
} // namespace firelight::achievements
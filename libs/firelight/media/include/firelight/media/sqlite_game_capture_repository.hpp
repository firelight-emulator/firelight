#pragma once

#include <firelight/media/game_capture_repository.hpp>

#include <memory>
#include <mutex>
#include <string>

namespace SQLite {
class Database;
}

namespace firelight::media {

// SQLite-backed capture index (captures.db)
class SqliteGameCaptureRepository final : public IGameCaptureRepository {
public:
  explicit SqliteGameCaptureRepository(std::string databaseFile);
  ~SqliteGameCaptureRepository() override;

  bool add(GameCapture &capture) override;
  std::vector<GameCapture> listAll() override;
  std::vector<GameCapture> listForGame(const std::string &contentHash) override;
  std::optional<GameCapture> getById(int id) override;
  bool setFavorite(int id, bool favorite) override;
  bool remove(int id) override;
  bool existsForPath(const std::string &filePath) override;

private:
  std::string m_databaseFile;
  std::unique_ptr<SQLite::Database> m_db;
  mutable std::recursive_mutex m_mutex;
};

} // namespace firelight::media

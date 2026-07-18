#pragma once

#include <firelight/metadata/game_metadata_source.hpp>

#include <memory>
#include <string>

namespace SQLite {
class Database;
}

namespace firelight::metadata {
/**
 * Read-only source for the game metadata database that gets shipped with the app
 */
class SqliteGameMetadataSource final : public IGameMetadataSource {
public:
  explicit SqliteGameMetadataSource(std::string databaseFile);

  ~SqliteGameMetadataSource() override;

  std::optional<GameMetadata> lookup(const std::string &contentHash) override;

private:
  std::string m_databaseFile;
  std::unique_ptr<SQLite::Database> m_db;
};
} // namespace firelight::metadata

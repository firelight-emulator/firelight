#pragma once

#include <firelight/cheats/cheat_repository.hpp>

#include <SQLiteCpp/Database.h>
#include <memory>
#include <string>

namespace firelight::cheats {

class SqliteCheatRepository final : public ICheatRepository {
public:
  explicit SqliteCheatRepository(std::string databaseFile);
  ~SqliteCheatRepository() override;

  std::vector<Cheat> getCheats(const std::string &contentHash) override;
  bool addCheat(Cheat &cheat) override;
  bool updateCheat(const Cheat &cheat) override;
  bool setEnabled(int id, bool enabled) override;
  bool removeCheat(int id) override;

private:
  std::string m_databaseFile;
  std::unique_ptr<SQLite::Database> m_database;
};

} // namespace firelight::cheats

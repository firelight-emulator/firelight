#pragma once

#include <SQLiteCpp/Database.h>
#include <firelight/settings/core_option_repository.hpp>

#include <memory>
#include <string>

namespace firelight::settings {

class SqliteCoreOptionRepository : public ICoreOptionRepository {
public:
  explicit SqliteCoreOptionRepository(std::string databaseFile);
  ~SqliteCoreOptionRepository() override;

  void upsertCoreOptions(const std::string &coreName,
                         const std::vector<CoreOption> &options) override;

  [[nodiscard]] std::vector<CoreOption>
  getCoreOptions(const std::string &coreName) override;

private:
  std::string m_databaseFile;
  std::unique_ptr<SQLite::Database> m_database;
};

} // namespace firelight::settings

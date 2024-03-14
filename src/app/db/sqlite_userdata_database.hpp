#pragma once

#include "userdata_database.hpp"
#include <QSqlDatabase>
#include <filesystem>
#include <vector>

namespace firelight::db {
class SqliteUserdataDatabase final : public IUserdataDatabase {
public:
  explicit SqliteUserdataDatabase(const std::filesystem::path &dbFile);
  ~SqliteUserdataDatabase() override;

  bool tableExists(std::string tableName) override;

  std::optional<SavefileMetadata> getSavefileMetadata(std::string contentMd5,
                                                      int slotNumber) override;
  bool updateSavefileMetadata(SavefileMetadata metadata) override;
  bool createSavefileMetadata(SavefileMetadata &metadata) override;
  std::vector<SavefileMetadata>
  getSavefileMetadataForContent(std::string contentMd5) override;

  bool createPlaySession(PlaySession &session) override;

private:
  std::filesystem::path m_database_path;
  QSqlDatabase m_database;
};

} // namespace firelight::db
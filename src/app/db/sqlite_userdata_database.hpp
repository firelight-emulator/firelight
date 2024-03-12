#pragma once

#include "userdata_database.hpp"
#include <QSqlDatabase>
#include <filesystem>
#include <vector>

namespace firelight::db {
class SqliteUserdataDatabase final : public IUserdataDatabase {
public:
  ~SqliteUserdataDatabase();
  bool tableExists(std::string tableName) override;
  std::optional<SavefileMetadata>
  getSavefileMetadata(std::string contentMd5, uint8_t slotNumber) override;
  void createPlaySession(PlaySession session) override;
  bool updateSavefileMetadata(SavefileMetadata metadata) override;
  bool createSavefileMetadata(SavefileMetadata &metadata) override;

  explicit SqliteUserdataDatabase(const std::filesystem::path &dbFile);
  std::vector<SavefileMetadata>
  getSavefileMetadataForContent(std::string contentMd5) override;
  ;

private:
  std::filesystem::path m_database_path;
  QSqlDatabase m_database;
};

} // namespace firelight::db
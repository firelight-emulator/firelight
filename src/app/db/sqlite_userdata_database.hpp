#pragma once

#include "firelight/userdata_database.hpp"
#include <QSqlDatabase>
#include <filesystem>
#include <activity/activity_log.hpp>

namespace firelight::db {
  class SqliteUserdataDatabase final : public IUserdataDatabase {
  public:
    explicit SqliteUserdataDatabase(const QString &dbFile);

    ~SqliteUserdataDatabase() override;

    bool tableExists(std::string tableName) override;

    std::optional<SavefileMetadata> getSavefileMetadata(std::string contentId,
                                                        int slotNumber) override;

    bool updateSavefileMetadata(SavefileMetadata metadata) override;

    bool createSavefileMetadata(SavefileMetadata &metadata) override;

    std::vector<SavefileMetadata>
    getSavefileMetadataForContent(std::string contentId) override;

    bool createSuspendPointMetadata(SuspendPointMetadata &metadata) override;

    std::optional<SuspendPointMetadata> getSuspendPointMetadata(std::string contentId, int saveSlotNumber,
                                                                int slotNumber) override;

    bool updateSuspendPointMetadata(const SuspendPointMetadata &metadata) override;

    std::vector<SuspendPointMetadata>
    getSuspendPointMetadataForContent(std::string contentId, int saveSlotNumber) override;

    bool deleteSuspendPointMetadata(int id) override;

    std::optional<std::string>
    getPlatformSettingValue(int platformId, std::string key) override;

    std::map<std::string, std::string> getAllPlatformSettings(int platformId) override;

    void setPlatformSettingValue(int platformId, std::string key, std::string value) override;

  private:
    QString m_database_path;
    QSqlDatabase m_database;
  };
} // namespace firelight::db

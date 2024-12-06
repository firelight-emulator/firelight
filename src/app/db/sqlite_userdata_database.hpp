#pragma once

#include "firelight/userdata_database.hpp"
#include <QSqlDatabase>
#include <filesystem>
#include <firelight/activity_log.hpp>

namespace firelight::db {
  class SqliteUserdataDatabase final : public IUserdataDatabase, public activity::IActivityLog {
  public:
    explicit SqliteUserdataDatabase(const std::filesystem::path &dbFile);

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

    bool createControllerProfile(ControllerProfile &profile) override;

    std::optional<ControllerProfile> getControllerProfile(int id) override;

    std::vector<ControllerProfile> getControllerProfiles() override;

    bool createPlaySession(PlaySession &session) override;

    std::optional<PlaySession>
    getLatestPlaySession(std::string contentId) override;

    std::optional<std::string>
    getPlatformSettingValue(int platformId, std::string key) override;

    std::map<std::string, std::string> getAllPlatformSettings(int platformId) override;

    void setPlatformSettingValue(int platformId, std::string key, std::string value) override;

  private:
    std::filesystem::path m_database_path;
    QSqlDatabase m_database;
  };
} // namespace firelight::db

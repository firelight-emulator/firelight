#pragma once

#include <firelight/saves/savefile_metadata.hpp>
#include <firelight/saves/suspend_point_metadata.hpp>

#include <optional>
#include <map>
#include <string>
#include <vector>

namespace firelight::db {
  class IUserdataDatabase {
  protected:
    virtual ~IUserdataDatabase() = default;

  public:
    /**
     * @param tableName The name of the table to check for.
     * @return true if the table exists, false otherwise.
     */
    virtual bool tableExists(std::string tableName) = 0;

    virtual bool createSavefileMetadata(saves::SavefileMetadata &metadata) = 0;

    virtual std::optional<saves::SavefileMetadata>
    getSavefileMetadata(std::string contentId, int slotNumber) = 0;

    virtual bool updateSavefileMetadata(saves::SavefileMetadata metadata) = 0;

    virtual std::vector<saves::SavefileMetadata>
    getSavefileMetadataForContent(std::string contentId) = 0;

    virtual bool createSuspendPointMetadata(saves::SuspendPointMetadata &metadata) = 0;

    virtual std::optional<saves::SuspendPointMetadata>
    getSuspendPointMetadata(std::string contentId, int saveSlotNumber, int slotNumber) = 0;

    virtual bool updateSuspendPointMetadata(const saves::SuspendPointMetadata &metadata) = 0;

    virtual std::vector<saves::SuspendPointMetadata>
    getSuspendPointMetadataForContent(std::string contentId, int saveSlotNumber) = 0;

    virtual bool deleteSuspendPointMetadata(int id) = 0;

    virtual std::optional<std::string> getPlatformSettingValue(int platformId, std::string key) = 0;

    virtual std::map<std::string, std::string> getAllPlatformSettings(int platformId) = 0;

    virtual void setPlatformSettingValue(int platformId, std::string key, std::string value) = 0;
  };
} // namespace firelight::db

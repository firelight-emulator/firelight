#pragma once

#include "play_session.hpp"
#include "savefile_metadata.hpp"
#include "suspend_point_metadata.hpp"

#include <optional>
#include <map>
#include <string>
#include <vector>

#include "controller_profile.hpp"

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

    virtual bool createSavefileMetadata(SavefileMetadata &metadata) = 0;

    virtual std::optional<SavefileMetadata>
    getSavefileMetadata(std::string contentId, int slotNumber) = 0;

    virtual bool updateSavefileMetadata(SavefileMetadata metadata) = 0;

    virtual std::vector<SavefileMetadata>
    getSavefileMetadataForContent(std::string contentId) = 0;

    virtual bool createSuspendPointMetadata(SuspendPointMetadata &metadata) = 0;

    virtual std::optional<SuspendPointMetadata>
    getSuspendPointMetadata(std::string contentId, int saveSlotNumber, int slotNumber) = 0;

    virtual bool updateSuspendPointMetadata(SuspendPointMetadata metadata) = 0;

    virtual std::vector<SuspendPointMetadata>
    getSuspendPointMetadataForContent(std::string contentId, int saveSlotNumber) = 0;

    virtual bool deleteSuspendPointMetadata(int id) = 0;

    virtual bool createControllerProfile(ControllerProfile &profile) = 0;

    virtual std::optional<ControllerProfile> getControllerProfile(int id) = 0;

    virtual std::vector<ControllerProfile> getControllerProfiles() = 0;

    virtual bool createPlaySession(PlaySession &session) = 0;

    virtual std::optional<PlaySession>
    getLatestPlaySession(std::string contentId) = 0;

    virtual std::optional<std::string> getPlatformSettingValue(int platformId, std::string key) = 0;

    virtual std::map<std::string, std::string> getAllPlatformSettings(int platformId) = 0;

    virtual void setPlatformSettingValue(int platformId, std::string key, std::string value) = 0;
  };
} // namespace firelight::db

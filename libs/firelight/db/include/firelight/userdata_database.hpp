#pragma once

#include <firelight/db/mod_installation.hpp>
#include <firelight/db/savefile_metadata.hpp>
#include <firelight/db/suspend_point_metadata.hpp>

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

    virtual bool createSavefileMetadata(SavefileMetadata &metadata) = 0;

    virtual std::optional<SavefileMetadata>
    getSavefileMetadata(std::string contentId, int slotNumber) = 0;

    virtual bool updateSavefileMetadata(SavefileMetadata metadata) = 0;

    virtual std::vector<SavefileMetadata>
    getSavefileMetadataForContent(std::string contentId) = 0;

    virtual bool createSuspendPointMetadata(SuspendPointMetadata &metadata) = 0;

    virtual std::optional<SuspendPointMetadata>
    getSuspendPointMetadata(std::string contentId, int saveSlotNumber, int slotNumber) = 0;

    virtual bool updateSuspendPointMetadata(const SuspendPointMetadata &metadata) = 0;

    virtual std::vector<SuspendPointMetadata>
    getSuspendPointMetadataForContent(std::string contentId, int saveSlotNumber) = 0;

    virtual bool deleteSuspendPointMetadata(int id) = 0;

    virtual std::optional<std::string> getPlatformSettingValue(int platformId, std::string key) = 0;

    virtual std::map<std::string, std::string> getAllPlatformSettings(int platformId) = 0;

    virtual void setPlatformSettingValue(int platformId, std::string key, std::string value) = 0;

    // Creates a mod installation row, assigning `installation.id` on success.
    virtual bool createModInstallation(ModInstallation &installation) = 0;

    // Persists mutable fields of an existing installation (installed version /
    // patch, save generation, patched hash, retained patch path, pin and
    // skip-version flags). Matches on `installation.id`.
    virtual bool updateModInstallation(const ModInstallation &installation) = 0;

    virtual std::optional<ModInstallation> getModInstallation(int id) = 0;

    // The single installation surfaced as the given library entry, if any.
    virtual std::optional<ModInstallation>
    getModInstallationForEntry(int entryId) = 0;

    // Every installation whose base game matches, for details-page aggregation.
    virtual std::vector<ModInstallation>
    getModInstallationsForBaseContentHash(std::string baseContentHash) = 0;

    // All installations, for the update-check sweep.
    virtual std::vector<ModInstallation> getAllModInstallations() = 0;

    virtual bool deleteModInstallation(int id) = 0;
  };
} // namespace firelight::db

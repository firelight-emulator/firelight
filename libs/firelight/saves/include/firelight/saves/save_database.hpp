#pragma once

#include <firelight/saves/savefile_metadata.hpp>
#include <firelight/saves/suspend_point_metadata.hpp>

#include <optional>
#include <string>
#include <vector>

namespace firelight::saves {

class ISaveDatabase {
public:
  virtual ~ISaveDatabase() = default;

  virtual bool createSavefileMetadata(SavefileMetadata &metadata) = 0;

  virtual std::optional<SavefileMetadata> getSavefileMetadata(std::string contentHash, int saveSlot) = 0;

  virtual bool updateSavefileMetadata(SavefileMetadata metadata) = 0;

  virtual std::vector<SavefileMetadata> getSavefileMetadataForContent(std::string contentHash) = 0;

  virtual bool createSuspendPointMetadata(SuspendPointMetadata &metadata) = 0;

  virtual std::optional<SuspendPointMetadata> getSuspendPointMetadata(std::string contentHash, int saveSlot,
                                                                      int pointIndex) = 0;

  virtual bool updateSuspendPointMetadata(const SuspendPointMetadata &metadata) = 0;

  virtual std::vector<SuspendPointMetadata> getSuspendPointMetadataForContent(std::string contentHash,
                                                                              int saveSlot) = 0;

  virtual bool transferContent(const std::string &fromContentHash, const std::string &toContentHash) = 0;

  virtual bool deleteSuspendPointMetadata(int id) = 0;
};
} // namespace firelight::saves

#pragma once

#include <firelight/saves/savefile.hpp>
#include <firelight/saves/savefile_info.hpp>
#include <firelight/saves/suspend_point.hpp>

#include <future>
#include <optional>
#include <string>
#include <vector>

namespace firelight::saves {

class ISaveManager {
public:
  virtual ~ISaveManager() = default;

  [[nodiscard]] virtual std::vector<SavefileInfo> getSaveFileInfoList(const std::string &contentHash) const = 0;

  virtual std::future<bool> writeSaveData(const std::string &contentHash, int saveSlotNumber,
                                          const Savefile &saveData) = 0;

  [[nodiscard]] virtual std::optional<Savefile> readSaveData(const std::string &contentHash,
                                                             int saveSlotNumber) const = 0;

  virtual void writeSuspendPoint(const std::string &contentHash, int saveSlotNumber, int index,
                                 const SuspendPoint &suspendPoint) = 0;

  virtual std::optional<SuspendPoint> readSuspendPoint(const std::string &contentHash, int saveSlotNumber,
                                                       int index) = 0;

  virtual void deleteSuspendPoint(const std::string &contentHash, int saveSlotNumber, int index) = 0;

  virtual bool transferSaves(const std::string &fromContentHash, const std::string &toContentHash) = 0;

  [[nodiscard]] virtual std::string getSaveDirectory() const = 0;

  // The one directory handed to cores for their own file writes
  [[nodiscard]] virtual std::string getSharedCoreSaveDirectory() const = 0;

  virtual void setSaveDirectory(const std::string &saveDirectory) = 0;
};
} // namespace firelight::saves

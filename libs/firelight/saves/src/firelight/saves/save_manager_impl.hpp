#pragma once

#include <firelight/saves/isave_manager.hpp>
#include <firelight/saves/save_database.hpp>

#include <string>

namespace firelight::saves {

class SaveManager final : public ISaveManager {
public:
  SaveManager(const std::string &saveDir, ISaveDatabase &saveDatabase);
  ~SaveManager() override;

  [[nodiscard]] std::vector<SavefileInfo> getSaveFileInfoList(const std::string &contentHash) const override;

  std::future<bool> writeSaveData(const std::string &contentHash, int saveSlotNumber,
                                  const Savefile &saveData) override;

  [[nodiscard]] std::optional<Savefile> readSaveData(const std::string &contentHash, int saveSlotNumber) const override;

  void writeSuspendPoint(const std::string &contentHash, int saveSlotNumber, int index,
                         const SuspendPoint &suspendPoint) override;

  std::optional<SuspendPoint> readSuspendPoint(const std::string &contentHash, int saveSlotNumber, int index) override;

  void deleteSuspendPoint(const std::string &contentHash, int saveSlotNumber, int index) override;

  bool transferSaves(const std::string &fromContentHash, const std::string &toContentHash) override;

  [[nodiscard]] std::string getSaveDirectory() const override;

  [[nodiscard]] std::string getSharedCoreSaveDirectory() const override;
  void setSaveDirectory(const std::string &saveDirectory) override;

private:
  void writeSuspendPointToDisk(const std::string &contentHash, int saveSlotNumber, int index,
                               const SuspendPoint &suspendPoint);

  [[nodiscard]] std::optional<SuspendPoint> readSuspendPointFromDisk(const std::string &contentHash, int saveSlotNumber,
                                                                     int index) const;

  void deleteSuspendPointFromDisk(const std::string &contentHash, int saveSlotNumber, int index);

  ISaveDatabase &m_saveDatabase;
  std::string m_saveDirectory;
};
} // namespace firelight::saves

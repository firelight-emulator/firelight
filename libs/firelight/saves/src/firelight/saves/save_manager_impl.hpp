#pragma once

#include <firelight/saves/isave_manager.hpp>
#include <firelight/saves/save_database.hpp>

#include <string>

namespace firelight::saves {
// Sqlite/filesystem-backed implementation of the ISaveManager domain contract
// Announces suspend-point changes through the EventDispatcher (see
// save_events.hpp) rather than Qt signals, so it is a plain, Qt-free class. The
// save directory is held in memory; persistence of that setting is the app
// layer's job (QtSaveManagerProxy), which passes the resolved directory in
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

  [[nodiscard]] std::string getSaveDirectory() const override;
  void setSaveDirectory(const std::string &saveDirectory) override;

private:
  void writeSuspendPointToDisk(const std::string &contentHash, int index, const SuspendPoint &suspendPoint);

  [[nodiscard]] std::optional<SuspendPoint> readSuspendPointFromDisk(const std::string &contentHash, int saveSlotNumber,
                                                                     int index) const;

  void deleteSuspendPointFromDisk(const std::string &contentHash, int saveSlotNumber, int index);

  ISaveDatabase &m_saveDatabase;
  std::string m_saveDirectory;
};
} // namespace firelight::saves

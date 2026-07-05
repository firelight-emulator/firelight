#pragma once

#include <firelight/saves/isave_manager.hpp>
#include <firelight/userdata_database.hpp>

#include <QSettings>
#include <memory>

namespace firelight::saves {
// Sqlite/filesystem-backed implementation of the ISaveManager domain contract.
// Announces suspend-point changes through the EventDispatcher (see
// save_events.hpp) rather than Qt signals, so it is a plain class, not a
// QObject. Its QSettings/QString/QDir internals are Qt value/utility types.
class SaveManager final : public ISaveManager {
public:
  SaveManager(const QString &defaultSaveDir,
              db::IUserdataDatabase &userdataDatabase);
  ~SaveManager() override;

  [[nodiscard]] std::vector<SavefileInfo>
  getSaveFileInfoList(const QString &contentHash) const override;

  std::future<bool> writeSaveData(const QString &contentHash,
                                  int saveSlotNumber,
                                  const Savefile &saveData) override;

  [[nodiscard]] std::optional<Savefile>
  readSaveData(const QString &contentHash, int saveSlotNumber) const override;

  void writeSuspendPoint(const QString &contentHash, int saveSlotNumber,
                         int index, const SuspendPoint &suspendPoint) override;

  std::optional<SuspendPoint> readSuspendPoint(const QString &contentHash,
                                               int saveSlotNumber,
                                               int index) override;

  void deleteSuspendPoint(const QString &contentHash, int saveSlotNumber,
                          int index) override;

  [[nodiscard]] QString getSaveDirectory() const override;
  void setSaveDirectory(const QString &saveDirectory) override;

private:
  void writeSuspendPointToDisk(const QString &contentHash, int index,
                               const SuspendPoint &suspendPoint);

  [[nodiscard]] std::optional<SuspendPoint>
  readSuspendPointFromDisk(const QString &contentHash, int saveSlotNumber,
                           int index) const;

  void deleteSuspendPointFromDisk(const QString &contentHash, int saveSlotNumber,
                                  int index);

  QSettings m_settings;
  db::IUserdataDatabase &m_userdataDatabase;
  QString m_saveDirectory;
};
} // namespace firelight::saves

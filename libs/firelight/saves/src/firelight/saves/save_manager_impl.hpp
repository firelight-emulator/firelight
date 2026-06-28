#pragma once

#include <firelight/saves/isave_manager.hpp>
#include <firelight/userdata_database.hpp>

#include <QSettings>
#include <QThreadPool>
#include <memory>

namespace firelight::saves {
class SaveManager final : public ISaveManager {
  Q_OBJECT

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

  QFuture<bool> writeSuspendPoint(const QString &contentHash, int saveSlotNumber,
                                  int index,
                                  const SuspendPoint &suspendPoint) override;

  std::optional<SuspendPoint> readSuspendPoint(const QString &contentHash,
                                               int saveSlotNumber,
                                               int index) override;

  void deleteSuspendPoint(const QString &contentHash, int saveSlotNumber,
                          int index) override;

  [[nodiscard]] QString getSaveDirectory() const override;
  void setSaveDirectory(const QString &saveDirectory) override;

public slots:
  void handleUpdatedSuspendPoint(int index);

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
  std::unique_ptr<QThreadPool> m_ioThreadPool = nullptr;
};
} // namespace firelight::saves

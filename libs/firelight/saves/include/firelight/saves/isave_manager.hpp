#pragma once

#include <firelight/saves/savefile.hpp>
#include <firelight/saves/savefile_info.hpp>
#include <firelight/saves/suspend_point.hpp>

#include <QFuture>
#include <QObject>
#include <QString>
#include <future>
#include <optional>
#include <vector>

namespace firelight::saves {
class ISaveManager : public QObject {
  Q_OBJECT
public:
  Q_PROPERTY(QString saveDirectory READ getSaveDirectory WRITE setSaveDirectory
                 NOTIFY saveDirectoryChanged)

  ~ISaveManager() override = default;

  [[nodiscard]] virtual std::vector<SavefileInfo>
  getSaveFileInfoList(const QString &contentHash) const = 0;

  virtual std::future<bool> writeSaveData(const QString &contentHash,
                                          int saveSlotNumber,
                                          const Savefile &saveData) = 0;

  [[nodiscard]] virtual std::optional<Savefile>
  readSaveData(const QString &contentHash, int saveSlotNumber) const = 0;

  virtual QFuture<bool>
  writeSuspendPoint(const QString &contentHash, int saveSlotNumber, int index,
                    const SuspendPoint &suspendPoint) = 0;

  virtual std::optional<SuspendPoint>
  readSuspendPoint(const QString &contentHash, int saveSlotNumber,
                   int index) = 0;

  virtual void deleteSuspendPoint(const QString &contentHash,
                                  int saveSlotNumber, int index) = 0;

  [[nodiscard]] virtual QString getSaveDirectory() const = 0;
  virtual void setSaveDirectory(const QString &saveDirectory) = 0;

signals:
  void saveDirectoryChanged(const QString &saveDirectory);
  void suspendPointUpdated(const QString &contentHash, int saveSlotNumber,
                           int index);
  void suspendPointDeleted(const QString &contentHash, int saveSlotNumber,
                           int index);
};
} // namespace firelight::saves

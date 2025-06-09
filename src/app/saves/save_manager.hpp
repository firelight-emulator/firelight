#pragma once

#include "../../gui/game_image_provider.hpp"
#include "firelight/userdata_database.hpp"
#include "library/library_entry.hpp"
#include "savefile.hpp"
#include "savefile_info.hpp"

#include <filesystem>
#include <firelight/content_database.hpp>
#include <library/library_database.hpp>
#include <map>
#include <qfuture.h>

#include "gui/suspend_point_list_model.hpp" // TODO: get rid of
#include "suspend_point.hpp"

#include <qdir.h>
#include <qsettings.h>

namespace firelight::saves {
class SaveManager final : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString saveDirectory READ getSaveDirectory WRITE setSaveDirectory
                 NOTIFY saveDirectoryChanged)

public:
  SaveManager(const QString &defaultSaveDir,
              db::IUserdataDatabase &userdataDatabase,
              gui::GameImageProvider &gameImageProvider);

  ~SaveManager() override;

  [[nodiscard]] std::vector<SavefileInfo>
  getSaveFileInfoList(const QString &contentHash) const;

  QFuture<bool> writeSaveData(const QString &contentHash, int saveSlotNumber,
                              const Savefile &saveData);

  [[nodiscard]] std::optional<Savefile> readSaveData(const QString &contentHash,
                                                     int saveSlotNumber) const;

  QFuture<bool> writeSuspendPoint(const QString &contentHash,
                                  int saveSlotNumber, int index,
                                  const SuspendPoint &suspendPoint);

  std::optional<SuspendPoint> readSuspendPoint(const QString &contentHash,
                                               int saveSlotNumber, int index);

  void deleteSuspendPoint(const QString &contentHash, int saveSlotNumber,
                          int index);

  [[nodiscard]] QString getSaveDirectory() const;

  void setSaveDirectory(const QString &saveDirectory);

public slots:
  void handleUpdatedSuspendPoint(int index);

signals:
  void saveDirectoryChanged(const QString &saveDirectory);
  void suspendPointUpdated(const QString &contentHash, int saveSlotNumber,
                           int index);
  void suspendPointDeleted(const QString &contentHash, int saveSlotNumber,
                           int index);

private:
  void writeSuspendPointToDisk(const QString &contentHash, int index,
                               const SuspendPoint &suspendPoint);

  [[nodiscard]] std::optional<SuspendPoint>
  readSuspendPointFromDisk(const QString &contentHash, int saveSlotNumber,
                           int index) const;

  void deleteSuspendPointFromDisk(const QString &contentHash,
                                  int saveSlotNumber, int index);

  QSettings m_settings;

  db::IUserdataDatabase &m_userdataDatabase;
  gui::GameImageProvider &m_gameImageProvider;
  QString m_saveDirectory;
  std::unique_ptr<QThreadPool> m_ioThreadPool = nullptr;
};
} // namespace firelight::saves

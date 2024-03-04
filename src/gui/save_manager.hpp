#pragma once

#include "../app/db/daos/lib_entry.hpp"
#include "../app/saves/save_data.hpp"
#include <filesystem>
#include <qfuture.h>

namespace firelight::Saves {

class SaveManager : public QObject {
  Q_OBJECT

signals:
  void saveDataChanged();
  void suspendPointCreated();
  void suspendPointLoaded();

public:
  explicit SaveManager(std::filesystem::path saveDir);
  QFuture<bool> writeSaveDataForEntry(LibEntry &entry,
                                      SaveData &saveData) const;
  std::optional<SaveData> readSaveDataForEntry(LibEntry &entry) const;

private:
  std::filesystem::path m_saveDir;
  std::unique_ptr<QThreadPool> m_ioThreadPool = nullptr;
};

} // namespace firelight::Saves

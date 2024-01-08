//
// Created by alexs on 1/7/2024.
//

#ifndef SAVE_MANAGER_HPP
#define SAVE_MANAGER_HPP
#include "../app/db/daos/lib_entry.hpp"
#include "../app/saves/save_data.hpp"
#include <QObject>
#include <QThreadPool>
#include <filesystem>

namespace Firelight::Saves {

class SaveManager : public QObject {
  Q_OBJECT

signals:
  void saveDataChanged();

public:
  explicit SaveManager(std::filesystem::path saveDir);
  void writeSaveDataForEntry(LibEntry &entry, SaveData &saveData) const;
  std::optional<SaveData> readSaveDataForEntry(LibEntry &entry) const;

private:
  std::filesystem::path m_saveDir;
  std::unique_ptr<QThreadPool> m_ioThreadPool = nullptr;
};

} // namespace Firelight::Saves

#endif // SAVE_MANAGER_HPP

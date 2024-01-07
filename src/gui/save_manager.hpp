//
// Created by alexs on 1/7/2024.
//

#ifndef SAVE_MANAGER_HPP
#define SAVE_MANAGER_HPP
#include "../app/db/daos/lib_entry.hpp"
#include "../app/saves/save_data.hpp"

#include <QObject>

namespace Firelight::Saves {

class SaveManager : public QObject {
  Q_OBJECT

signals:
  void saveDataChanged();

public:
  void writeSaveDataForEntry(LibEntry &entry, SaveData &saveData);
  std::optional<SaveData> readSaveDataForEntry(LibEntry &entry);
};

} // namespace Firelight::Saves

#endif // SAVE_MANAGER_HPP

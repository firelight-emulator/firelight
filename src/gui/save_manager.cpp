//
// Created by alexs on 1/7/2024.
//

#include "save_manager.hpp"

#include <spdlog/spdlog.h>

namespace Firelight::Saves {

void SaveManager::writeSaveDataForEntry(LibEntry &entry, SaveData &saveData) {
  spdlog::info("writing save data for entry {}", entry.content_path);
}
std::optional<SaveData> SaveManager::readSaveDataForEntry(LibEntry &entry) {
  return {};
}

} // namespace Firelight::Saves
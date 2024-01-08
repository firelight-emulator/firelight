//
// Created by alexs on 1/7/2024.
//

#include "save_manager.hpp"

#include <fstream>
#include <spdlog/spdlog.h>

#include <utility>

namespace Firelight::Saves {

SaveManager::SaveManager(std::filesystem::path saveDir)
    : m_saveDir(std::move(saveDir)) {}

void SaveManager::writeSaveDataForEntry(LibEntry &entry,
                                        const SaveData &saveData) const {
  const auto directory = m_saveDir / entry.md5;
  create_directories(directory);

  const auto saveFile = directory / "savedata.sram";

  std::ofstream saveFileStream(saveFile, std::ios::binary);
  saveFileStream.write(
      reinterpret_cast<const char *>(saveData.getSaveRamData().data()),
      saveData.getSaveRamData().size());
  saveFileStream.close();

  spdlog::info("writing save data for entry {}", entry.content_path);
}

std::optional<SaveData>
SaveManager::readSaveDataForEntry(LibEntry &entry) const {
  const auto directory = m_saveDir / entry.md5;
  if (!exists(directory)) {
    return {};
  }

  const auto saveFile = directory / "savedata.sram";
  const auto fileSize = file_size(saveFile);
  std::ifstream saveFileStream(saveFile, std::ios::binary);

  std::vector<std::byte> fileContents(fileSize);
  saveFileStream.read(reinterpret_cast<char *>(fileContents.data()), fileSize);
  saveFileStream.close();

  SaveData saveData(fileContents);

  return {saveData};
}

} // namespace Firelight::Saves
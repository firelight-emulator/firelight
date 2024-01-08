//
// Created by alexs on 1/7/2024.
//

#include "save_manager.hpp"
#include <fstream>
#include <spdlog/spdlog.h>

namespace Firelight::Saves {

SaveManager::SaveManager(std::filesystem::path saveDir)
    : m_saveDir(std::move(saveDir)) {}

void SaveManager::writeSaveDataForEntry(LibEntry &entry,
                                        const SaveData &saveData) const {
  const auto directory = m_saveDir / entry.md5;
  create_directories(directory);

  const auto saveFile = directory / "savedata.sram";

  std::ofstream saveFileStream(saveFile, std::ios::binary);
  saveFileStream.write(saveData.getSaveRamData().data(),
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

  std::ifstream saveFileStream(saveFile, std::ios::binary);

  auto size = file_size(saveFile);

  std::vector<char> data(size);

  saveFileStream.read(data.data(), size);
  saveFileStream.close();

  auto fileContents = std::vector(data.data(), data.data() + size);

  SaveData saveData(fileContents);

  return {saveData};
}
std::vector<char> SaveManager::getStuff(LibEntry &entry) const {
  const auto directory = m_saveDir / entry.md5;
  if (!exists(directory)) {
    return {};
  }

  const auto saveFile = directory / "savedata.sram";

  std::ifstream saveFileStream(saveFile, std::ios::binary);

  auto size = file_size(saveFile);

  std::vector<char> data(size);

  saveFileStream.read(data.data(), size);
  saveFileStream.close();

  return data;
}

} // namespace Firelight::Saves
//
// Created by alexs on 1/7/2024.
//

#include "save_manager.hpp"
#include <fstream>
#include <qfuture.h>
#include <qtconcurrentrun.h>
#include <spdlog/spdlog.h>

namespace Firelight::Saves {

SaveManager::SaveManager(std::filesystem::path saveDir)
    : m_saveDir(std::move(saveDir)) {
  m_ioThreadPool = std::make_unique<QThreadPool>();
  m_ioThreadPool->setMaxThreadCount(1);
}

void SaveManager::writeSaveDataForEntry(LibEntry &entry,
                                        SaveData &saveData) const {
  const auto directory = m_saveDir / entry.md5;
  create_directories(directory);

  const auto saveFile = directory / "savedata.sram";

  std::ofstream saveFileStream(saveFile, std::ios::binary);
  saveFileStream.write(saveData.getSaveRamData().data(),
                       saveData.getSaveRamData().size());
  saveFileStream.close();

  const auto screenshotFile = directory / "screenshot.png";
  if (!saveData.getImage().save(
          QString::fromStdString(screenshotFile.string()))) {
    spdlog::warn("Unable to save screenshot for entry {}", entry.content_path);
  }

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

} // namespace Firelight::Saves
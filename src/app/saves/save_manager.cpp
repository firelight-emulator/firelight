//
// Created by alexs on 11/18/2023.
//

#include "save_manager.hpp"
#include <fstream>

namespace FL {

const std::filesystem::path BASE_USER_DIR = "./userdata";

std::filesystem::path SaveManager::getSaveFileDirectory() {
  return BASE_USER_DIR / profileId / "saves";
}

std::filesystem::path SaveManager::getSaveFilePath(const std::string &gameId) {
  std::filesystem::path path = getSaveFileDirectory() / gameId;
  path.replace_extension(".save");

  return path;
}

std::vector<char> SaveManager::read(const std::string &gameId) {
  std::filesystem::path path(getSaveFilePath(gameId));
  std::ifstream saveFile(path, std::ios::binary);
  if (saveFile) {
    auto size = file_size(path);

    char data[size];
    saveFile.read(data, size);
    saveFile.close();

    return std::vector<char>(data, data + sizeof data / sizeof data[0]);
  }

  return {};
}

void SaveManager::write(const std::string &gameId, std::vector<char> data) {
  std::filesystem::create_directories(getSaveFileDirectory());
  std::ofstream saveFile(getSaveFilePath(gameId), std::ios::binary);
  saveFile.write(data.data(), data.size());
  saveFile.close();
}

} // namespace FL
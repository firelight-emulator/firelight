//
// Created by alexs on 1/10/2024.
//

#include "game_loader.hpp"

#include <fstream>
#include <spdlog/spdlog.h>

namespace Firelight {
void GameLoader::loadGame(int entryId) {
  spdlog::info("Loading entry with id {}", entryId);
  auto entry = getLibraryManager()->get_by_id(entryId);

  if (!entry.has_value()) {
    emit gameLoadFailedNoSuchEntry(entryId);
    spdlog::warn("Entry with id {} does not exist...", entryId);
  }

  if (!std::filesystem::exists(entry->content_path)) {
    emit gameLoadFailedContentMissing(entryId);
    return;
  }

  std::string corePath;
  if (entry->platform == 0) {
    corePath = "./system/_cores/mupen64plus_next_libretro.dll";
  } else if (entry->platform == 1) {
    corePath = "./system/_cores/snes9x_libretro.dll";
  }

  auto size = std::filesystem::file_size(entry->content_path);

  QByteArray saveDataBytes;
  const auto saveData = getSaveManager()->readSaveDataForEntry(*entry);
  if (saveData.has_value()) {
    saveDataBytes = QByteArray(saveData->getSaveRamData().data(),
                               saveData->getSaveRamData().size());
  }

  if (entry->type == EntryType::ROMHACK) {
    emit gameLoadFailedOrphanedPatch(entryId);
    return;
  }

  QByteArray gameData;
  std::ifstream file(entry->content_path, std::ios::binary);

  file.read(gameData.data(), size);
  file.close();

  emit gameLoaded(entryId, gameData, saveDataBytes,
                  QString::fromStdString(corePath));
}
} // namespace Firelight
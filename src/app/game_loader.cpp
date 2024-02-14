//
// Created by alexs on 1/10/2024.
//

#include "game_loader.hpp"

#include "patching/ips_patch.hpp"
#include "patching/pm_star_rod_mod_patch.hpp"
#include "patching/yay_0_codec.hpp"

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
  } else if (entry->platform == 2) {
    corePath = "./system/_cores/gambatte_libretro.dll";
  } else if (entry->platform == 3) {
    corePath = "./system/_cores/gambatte_libretro.dll";
  }

  auto size = std::filesystem::file_size(entry->content_path);

  QByteArray saveDataBytes;
  const auto saveData = getSaveManager()->readSaveDataForEntry(*entry);
  if (saveData.has_value()) {
    saveDataBytes = QByteArray(saveData->getSaveRamData().data(),
                               saveData->getSaveRamData().size());
  }

  QByteArray gameData;

  if (entry->type == EntryType::PATCH) {
    if (entry->parent_entry == -1) {
      // TODO: This is probably where we should try to find the rom in the
      // library
      emit gameLoadFailedOrphanedPatch(entryId);
      return;
    }

    auto romEntry = getLibraryManager()->get_by_id(entry->parent_entry);
    if (!romEntry.has_value()) {
      emit gameLoadFailedOrphanedPatch(entryId);
      return;
    }

    if (romEntry->platform == 0) {
      corePath = "./system/_cores/mupen64plus_next_libretro.dll";
    } else if (romEntry->platform == 1) {
      corePath = "./system/_cores/snes9x_libretro.dll";
    } else if (romEntry->platform == 2) {
      corePath = "./system/_cores/gambatte_libretro.dll";
    } else if (romEntry->platform == 3) {
      corePath = "./system/_cores/gambatte_libretro.dll";
    }

    size = std::filesystem::file_size(entry->content_path);
    std::vector<uint8_t> patchData(size);
    std::ifstream file(entry->content_path, std::ios::binary);
    // std::ifstream file(entry->content_path, std::ios::binary);

    file.read(reinterpret_cast<char *>(patchData.data()), size);
    file.close();

    // gameData = QByteArray(gameDataVec.data(), gameDataVec.size());

    auto ext = std::filesystem::path(entry->content_path).extension();

    if (ext == ".mod") {
      FL::Patching::Yay0Codec codec;

      auto decompressed = codec.decompress(patchData.data());

      FL::Patching::PMStarRodModPatch patch(decompressed);

      std::filesystem::path gamePath = romEntry->content_path;
      std::ifstream game(gamePath, std::ios::binary);

      auto gameSize = file_size(gamePath);
      std::vector<uint8_t> romData(gameSize);

      game.read(reinterpret_cast<char *>(romData.data()), gameSize);

      auto patchedGame = patch.patchRom(romData);
      gameData = QByteArray(reinterpret_cast<char *>(patchedGame.data()),
                            patchedGame.size());
    } else if (ext == ".ips") {
      Patching::IPSPatch patch(patchData);

      std::filesystem::path gamePath = romEntry->content_path;
      std::ifstream game(gamePath, std::ios::binary);

      auto gameSize = file_size(gamePath);
      std::vector<uint8_t> romData(gameSize);

      game.read(reinterpret_cast<char *>(romData.data()), gameSize);

      auto patchedGame = patch.patchRom(romData);
      gameData = QByteArray(reinterpret_cast<char *>(patchedGame.data()),
                            patchedGame.size());
    }

    // Check if the rom field has a value
    // If so, check the library for it
    // If it's there, load the content from that entry and patch it
    // If it's not, prompt the user to select a rom to patch
  } else {
    std::vector<char> gameDataVec(size);
    std::ifstream file(entry->content_path, std::ios::binary);

    file.read(gameDataVec.data(), size);
    file.close();

    gameData = QByteArray(gameDataVec.data(), gameDataVec.size());
  }

  emit gameLoaded(entryId, gameData, saveDataBytes,
                  QString::fromStdString(corePath));
}
} // namespace Firelight
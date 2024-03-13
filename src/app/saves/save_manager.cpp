#include "save_manager.hpp"

#include "../db/daos/library_entry.hpp"
#include "../util/md5.hpp"

#include <fstream>
#include <qfuture.h>
#include <qtconcurrentrun.h>
#include <spdlog/spdlog.h>

namespace firelight::Saves {

SaveManager::SaveManager(std::filesystem::path saveDir,
                         db::IUserdataDatabase &userdataDatabase)
    : m_userdataDatabase(userdataDatabase), m_saveDir(std::move(saveDir)) {
  m_ioThreadPool = std::make_unique<QThreadPool>();
  m_ioThreadPool->setMaxThreadCount(1);
}

QFuture<bool> SaveManager::writeSaveDataForEntry(db::LibraryEntry &entry,
                                                 Savefile &saveData) const {
  return QtConcurrent::run([this, entry, saveData] {
    // TODO: Add some verification that the metadata is correct
    // TODO: Save file could have been deleted, etc
    auto slot = 1;

    auto exists = false;
    db::SavefileMetadata metadata;

    auto metadataOpt =
        m_userdataDatabase.getSavefileMetadata(entry.contentMd5, slot);

    if (metadataOpt.has_value()) {
      metadata = metadataOpt.value();
      exists = true;
    } else {
      metadata.contentMd5 = entry.contentMd5;
      metadata.slotNumber = slot;
    }

    const auto bytes = saveData.getSaveRamData();

    // TODO: Calculate actual savefile MD5
    auto savefileMd5 = calculateMD5(bytes);

    if (metadata.savefileMd5 == savefileMd5) {
      return true;
    }

    spdlog::info("Writing updated savefile for {}", entry.contentMd5);
    metadata.savefileMd5 = savefileMd5;

    const auto directory =
        m_saveDir / metadata.contentMd5 / ("slot" + std::to_string(slot));
    create_directories(directory);

    const auto saveFile = directory / "savefile.sram";

    std::ofstream saveFileStream(saveFile, std::ios::binary);
    saveFileStream.write(bytes.data(), bytes.size());
    saveFileStream.close();

    auto timestamp = 0;
    metadata.lastModifiedAt = timestamp;

    if (exists) {
      m_userdataDatabase.updateSavefileMetadata(metadata);
    } else {
      m_userdataDatabase.createSavefileMetadata(metadata);
    }

    // if (image != nullptr) {
    // }
    // const auto screenshotFile = directory / "screenshot.png";
    // if (!image.save(QString::fromStdString(screenshotFile.string()))) {
    //   spdlog::warn("Unable to save screenshot for entry {}", md5);
    // }

    // spdlog::info("writing save data for entry {}", md5);
    return true;
  });

  // const auto directory = m_saveDir / entry.md5;
  // create_directories(directory);
  //
  // const auto saveFile = directory / "savedata.sram";
  //
  // std::ofstream saveFileStream(saveFile, std::ios::binary);
  // saveFileStream.write(saveData.getSaveRamData().data(),
  //                      saveData.getSaveRamData().size());
  // saveFileStream.close();
  //
  // const auto screenshotFile = directory / "screenshot.png";
  // if (!saveData.getImage().save(
  //         QString::fromStdString(screenshotFile.string()))) {
  //   spdlog::warn("Unable to save screenshot for entry {}",
  //   entry.content_path);
  // }
  //
  // spdlog::info("writing save data for entry {}", entry.content_path);
}

std::optional<Savefile>
SaveManager::readSaveDataForEntry(db::LibraryEntry &entry) const {
  const auto directory = m_saveDir / entry.contentMd5 / "slot1";
  if (!exists(directory)) {
    return std::nullopt;
  }

  const auto saveFile = directory / "savefile.sram";

  std::ifstream saveFileStream(saveFile, std::ios::binary);

  auto size = file_size(saveFile);

  std::vector<char> data(size);

  saveFileStream.read(data.data(), size);
  saveFileStream.close();

  auto fileContents = std::vector(data.data(), data.data() + size);

  Savefile saveData(fileContents);

  return {saveData};
}

} // namespace firelight::Saves
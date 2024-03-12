#include "save_manager.hpp"

#include "../db/daos/library_entry.hpp"

#include <fstream>
#include <qfuture.h>
#include <qtconcurrentrun.h>

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

    auto metadata =
        m_userdataDatabase.getSavefileMetadata(entry.contentMd5, slot).value();
    // m_userdataDatabase.getOrCreateSavefileMetadata(entry.contentMd5, slot);

    const auto bytes = saveData.getSaveRamData();
    auto savefileMd5 = "heytyhere";

    if (metadata.savefileMd5 == savefileMd5) {
      return true;
    }

    metadata.savefileMd5 = savefileMd5;

    // auto image = saveData.getImage();
    // spdlog::info("md5: {}, bytesLength: {}, imageWidth: {}, imageHeight: {}",
    //              md5, bytes.size(), image.width(), image.height());
    // return true;
    const auto directory =
        m_saveDir / entry.contentMd5 / ("slot" + std::to_string(slot));
    create_directories(directory);

    const auto saveFile = directory / "savefile.sram";

    std::ofstream saveFileStream(saveFile, std::ios::binary);
    saveFileStream.write(bytes.data(), bytes.size());
    saveFileStream.close();

    auto timestamp = 0;
    metadata.lastModifiedAt = timestamp;

    m_userdataDatabase.updateSavefileMetadata(metadata);

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
  const auto directory = m_saveDir / entry.contentMd5;
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

  Savefile saveData(fileContents);

  return {saveData};
}

} // namespace firelight::Saves
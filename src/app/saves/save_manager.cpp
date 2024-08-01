#include "save_manager.hpp"

#include "../util/md5.hpp"
#include "firelight/library_entry.hpp"

#include <fstream>
#include <qfuture.h>
#include <qtconcurrentrun.h>
#include <spdlog/spdlog.h>

namespace firelight::saves {
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
      auto slot = entry.activeSaveSlot;

      auto exists = false;
      db::SavefileMetadata metadata;

      auto metadataOpt =
          m_userdataDatabase.getSavefileMetadata(entry.contentId, slot);

      if (metadataOpt.has_value()) {
        // printf("Metadata exists\n");
        metadata = metadataOpt.value();
        exists = true;
      } else {
        // printf("Metadata DOES NOT exist\n");
        metadata.contentId = entry.contentId;
        metadata.slotNumber = slot;
      }

      const auto bytes = saveData.getSaveRamData();

      auto savefileMd5 = calculateMD5(bytes);

      if (metadata.savefileMd5 == savefileMd5) {
        return true;
      }

      spdlog::info("Writing updated savefile for {} slot {}", entry.contentId,
                   slot);
      metadata.savefileMd5 = savefileMd5;

      const auto directory =
          m_saveDir / metadata.contentId / ("slot" + std::to_string(slot));
      create_directories(directory);

      const auto tempSaveFile = directory / "savefile.srm.tmp";
      const auto saveFile = directory / "savefile.srm";

      std::ofstream saveFileStream(tempSaveFile, std::ios::binary);
      saveFileStream.write(bytes.data(), bytes.size());
      saveFileStream.close();

      std::filesystem::rename(tempSaveFile, saveFile);

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
  }

  std::optional<Savefile>
  SaveManager::readSaveDataForEntry(db::LibraryEntry &entry) const {
    auto slot = entry.activeSaveSlot;

    const auto directory =
        m_saveDir / entry.contentId / ("slot" + std::to_string(slot));
    if (!exists(directory)) {
      return std::nullopt;
    }

    const auto saveFile = directory / "savefile.srm";

    printf("Reading from save file: %s\n", saveFile.string().c_str());

    if (!exists(saveFile)) {
      return std::nullopt;
    }

    std::ifstream saveFileStream(saveFile, std::ios::binary);

    auto size = file_size(saveFile);

    std::vector<char> data(size);

    saveFileStream.read(data.data(), size);
    saveFileStream.close();

    auto fileContents = std::vector(data.data(), data.data() + size);

    Savefile saveData(fileContents);

    return {saveData};
  }
} // namespace firelight::saves

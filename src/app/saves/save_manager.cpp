#include "save_manager.hpp"

#include "firelight/library_entry.hpp"

#include <fstream>
#include <qcryptographichash.h>
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

      auto savefileMd5 = QCryptographicHash::hash(bytes, QCryptographicHash::Md5).toHex().toStdString();

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

    spdlog::info("Reading from save file: {}", saveFile.string());

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

  QFuture<bool> SaveManager::writeSuspendPointForEntry(db::LibraryEntry &entry, int index,
                                                       SuspendPoint &suspendPoint) const {
    printf("lol I aint doing jack buddy\n");
    return {};
  }

  std::optional<SuspendPoint> SaveManager::readSuspendPointForEntry(db::LibraryEntry &entry, int index) const {
    return {};
  }
} // namespace firelight::saves

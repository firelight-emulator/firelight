#include "save_manager.hpp"

#include "firelight/library_entry.hpp"

#include <fstream>
#include <qcryptographichash.h>
#include <qfile.h>
#include <qfuture.h>
#include <qtconcurrentrun.h>
#include <spdlog/spdlog.h>

namespace firelight::saves {
  SaveManager::SaveManager(std::filesystem::path saveDir,
                           db::ILibraryDatabase &libraryDatabase,
                           db::IUserdataDatabase &userdataDatabase,
                           gui::GameImageProvider &gameImageProvider)
    : m_userdataDatabase(userdataDatabase), m_libraryDatabase(libraryDatabase), m_gameImageProvider(gameImageProvider),
      m_saveDir((std::move
          (saveDir)
        )
      ) {
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

  QFuture<bool> SaveManager::writeSuspendPointForEntry(const db::LibraryEntry &entry, const int index,
                                                       const SuspendPoint &suspendPoint) {
    if (entry.id == m_currentSuspendPointListEntryId) {
      m_suspendPointListModel->updateData(index, suspendPoint);
    }

    // m_suspendPoints[index] = suspendPoint;
    writeSuspendPointToDisk(entry, index, suspendPoint);
    return {};
  }

  std::optional<SuspendPoint> SaveManager::readSuspendPointForEntry(db::LibraryEntry &entry, int saveSlotNumber,
                                                                    const int index) {
    if (m_suspendPoints.contains(index)) {
      return m_suspendPoints.at(index);
    }

    return {};
  }

  QAbstractListModel *SaveManager::getSuspendPointListModel(const int entryId, int saveSlotNumber) {
    if (entryId == m_currentSuspendPointListEntryId && m_suspendPointListModel != nullptr) {
      return m_suspendPointListModel.get();
    }

    if (m_suspendPointListModel != nullptr) {
      m_suspendPointListModel.reset();
      m_currentSuspendPointListEntryId = -1;
      m_currentSuspendPointListSaveSlotNumber = -1;
    }

    m_currentSuspendPointListEntryId = entryId;
    m_currentSuspendPointListSaveSlotNumber = saveSlotNumber;
    m_suspendPointListModel = std::make_unique<emulation::SuspendPointListModel>(m_gameImageProvider, this);
    connect(m_suspendPointListModel.get(), &emulation::SuspendPointListModel::suspendPointUpdated,
            this, &SaveManager::handleUpdatedSuspendPoint);

    for (int i = 0; i < 8; ++i) {
      auto p = readSuspendPointFromDisk(entryId, saveSlotNumber, i);
      if (p.has_value()) {
        m_suspendPoints[i] = p.value();
        m_suspendPointListModel->updateData(i, p.value());
      }
    }

    return m_suspendPointListModel.get();
  }

  void SaveManager::clearSuspendPointListModel() {
    m_suspendPointListModel.reset();
    m_currentSuspendPointListEntryId = -1;
  }

  void SaveManager::handleUpdatedSuspendPoint(int index) {
    auto slotNumber = index + 1;
    if (m_currentSuspendPointListEntryId == -1 || m_currentSuspendPointListSaveSlotNumber == -1) {
      return;
    }

    auto item = m_suspendPointListModel->getItem(index);
    if (!item.has_value()) {
      return;
    }

    if (!item->hasData) {
      deleteSuspendPointFromDisk(m_currentSuspendPointListEntryId, m_currentSuspendPointListSaveSlotNumber, index);
      return;
    }

    auto entry = m_libraryDatabase.getLibraryEntry(m_currentSuspendPointListEntryId);
    if (!entry.has_value()) {
      return;
    }

    auto metadata = m_userdataDatabase.getSuspendPointMetadata(entry->contentId,
                                                               m_currentSuspendPointListSaveSlotNumber, slotNumber);
    if (metadata.has_value()) {
      metadata->lastModifiedAt = QDateTime::currentMSecsSinceEpoch();
      metadata->locked = item->locked;
      m_userdataDatabase.updateSuspendPointMetadata(*metadata);
    } else {
      auto ms = QDateTime::currentMSecsSinceEpoch();
      db::SuspendPointMetadata newMetadata{
        .contentId = entry->contentId,
        .saveSlotNumber = static_cast<unsigned int>(m_currentSuspendPointListSaveSlotNumber),
        .slotNumber = static_cast<unsigned int>(slotNumber),
        .lastModifiedAt = static_cast<uint64_t>(ms),
        .createdAt = static_cast<uint64_t>(ms),
        .locked = item->locked
      };

      m_userdataDatabase.createSuspendPointMetadata(newMetadata);
    }

    metadata->locked = item->locked;

    m_userdataDatabase.updateSuspendPointMetadata(*metadata);
  }

  void SaveManager::writeSuspendPointToDisk(const db::LibraryEntry &entry, int index,
                                            const SuspendPoint &suspendPoint) {
    unsigned int slotNumber = index + 1;
    if (suspendPoint.locked) {
      spdlog::warn("Trying to write locked suspend point for entry with id {} and index {}",
                   entry.id, index);
      return;
    }

    const auto directory =
        m_saveDir / entry.contentId / ("slot" + std::to_string(suspendPoint.saveSlotNumber)) / "suspendpoints" / (
          "slot" + std::to_string(slotNumber));

    if (!exists(directory)) {
      create_directories(directory);
    }

    const auto tempSaveFile = directory / "suspendpoint.state.tmp";
    const auto saveFile = directory / "suspendpoint.state";

    std::ofstream saveFileStream(tempSaveFile, std::ios::binary);
    saveFileStream.write(reinterpret_cast<const char *>(suspendPoint.state.data()), suspendPoint.state.size());
    saveFileStream.close();

    std::filesystem::rename(tempSaveFile, saveFile);

    if (!suspendPoint.image.isNull()) {
      auto imageFilename = directory / "screenshot.png";
      suspendPoint.image.save(QString::fromStdString(imageFilename.string()), "PNG");
    }

    if (!suspendPoint.retroachievementsState.empty()) {
      const auto tempRcheevosFile = directory / "rcheevos.state.tmp";
      const auto rcheevosFile = directory / "rcheevos.state";

      std::ofstream rcheevosStream(tempRcheevosFile, std::ios::binary);
      rcheevosStream.write(reinterpret_cast<const char *>(suspendPoint.retroachievementsState.data()),
                           suspendPoint.retroachievementsState.size());
      rcheevosStream.close();

      std::filesystem::rename(tempRcheevosFile, rcheevosFile);
    }

    auto metadata = m_userdataDatabase.
        getSuspendPointMetadata(entry.contentId, suspendPoint.saveSlotNumber, slotNumber);
    if (metadata.has_value()) {
      metadata->lastModifiedAt = QDateTime::currentMSecsSinceEpoch();
      metadata->locked = suspendPoint.locked;
      m_userdataDatabase.updateSuspendPointMetadata(*metadata);
    } else {
      auto ms = QDateTime::currentMSecsSinceEpoch();
      db::SuspendPointMetadata newMetadata{
        .contentId = entry.contentId,
        .saveSlotNumber = suspendPoint.saveSlotNumber,
        .slotNumber = slotNumber,
        .lastModifiedAt = static_cast<uint64_t>(ms),
        .createdAt = static_cast<uint64_t>(ms),
        .locked = suspendPoint.locked
      };

      m_userdataDatabase.createSuspendPointMetadata(newMetadata);
    }

    // auto timestamp = 0;
    // metadata.lastModifiedAt = timestamp;
    //
    // if (exists) {
    //   m_userdataDatabase.updateSavefileMetadata(metadata);
    // } else {
    //   m_userdataDatabase.createSavefileMetadata(metadata);
    // }
  }

  std::optional<SuspendPoint> SaveManager::readSuspendPointFromDisk(int entryId, int saveSlotNumber, int index) {
    auto slotNumber = index + 1;
    auto entry = m_libraryDatabase.getLibraryEntry(entryId);
    if (!entry.has_value()) {
      return std::nullopt;
    }

    // TODO: metadata

    auto path = m_saveDir / entry->contentId / ("slot" + std::to_string(saveSlotNumber)) / "suspendpoints" / (
                  "slot" + std::to_string(slotNumber));
    if (!exists(path)) {
      return std::nullopt;
    }

    auto stateFile = path / "suspendpoint.state";
    if (!exists(stateFile)) {
      return std::nullopt;
    }

    std::ifstream saveFileStream(stateFile, std::ios::binary);

    auto size = file_size(stateFile);

    std::vector<uint8_t> data(size);

    saveFileStream.read(reinterpret_cast<char *>(data.data()), size);
    saveFileStream.close();

    auto fileContents = std::vector(data.data(), data.data() + size);

    QImage image;
    if (exists(path / "screenshot.png")) {
      image.load(QString::fromStdString((path / "screenshot.png").string()));
    }

    std::vector<uint8_t> rcheevosData;
    if (auto rcheevosFile = path / "rcheevos.state"; exists(rcheevosFile)) {
      QFile file(QString::fromStdString(rcheevosFile.string()));
      file.open(QIODeviceBase::ReadOnly);
      auto bytes = file.readAll();
      rcheevosData = std::vector<uint8_t>(bytes.begin(), bytes.end());
      file.close();
    }

    printf("Size of rcheevosData: %llu\n", rcheevosData.size());


    long long created = 0;
    bool locked = false;

    auto metadata = m_userdataDatabase.getSuspendPointMetadata(entry->contentId, saveSlotNumber, slotNumber);
    if (metadata.has_value()) {
      created = metadata->createdAt;
      locked = metadata->locked;
    }

    return SuspendPoint{
      .state = fileContents,
      .retroachievementsState = rcheevosData,
      .timestamp = created,
      .image = image,
      .locked = locked,
    };
  }

  void SaveManager::deleteSuspendPointFromDisk(int entryId, int saveSlotNumber, int index) {
    auto slotNumber = index + 1;
    auto entry = m_libraryDatabase.getLibraryEntry(entryId);
    if (!entry.has_value()) {
      return;
    }

    auto path = m_saveDir / entry->contentId / ("slot" + std::to_string(saveSlotNumber)) / "suspendpoints" / (
                  "slot" + std::to_string(slotNumber));

    if (!exists(path)) {
      return;
    }

    auto metadata = m_userdataDatabase.getSuspendPointMetadata(entry->contentId, saveSlotNumber, slotNumber);
    if (metadata.has_value()) {
      m_userdataDatabase.deleteSuspendPointMetadata(metadata->id);
    }

    // TODO: Delete metadata
    printf("Deleting suspend point from disk\n");

    std::error_code errorCode;
    std::filesystem::remove(path / "suspendpoint.state", errorCode);
    if (errorCode.value()) {
      printf("ERROR CODE VALUE: %d, description: %s\n", errorCode.value(), errorCode.message().c_str());
    }

    std::filesystem::remove(path / "screenshot.png", errorCode);
    if (errorCode.value()) {
      printf("ERROR CODE VALUE: %d, description: %s\n", errorCode.value(), errorCode.message().c_str());
    }

    std::filesystem::remove(path / "rcheevos.state", errorCode);
    if (errorCode.value()) {
      printf("ERROR CODE VALUE: %d, description: %s\n", errorCode.value(), errorCode.message().c_str());
    }
  }
} // namespace firelight::saves

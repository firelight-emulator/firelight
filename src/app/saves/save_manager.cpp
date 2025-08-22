#include "save_manager.hpp"

#include "library/library_entry.hpp"

#include <fstream>
#include <qcryptographichash.h>
#include <qfile.h>
#include <qfuture.h>
#include <qsavefile.h>
#include <qtconcurrentrun.h>
#include <spdlog/spdlog.h>

namespace firelight::saves {
SaveManager::SaveManager(const QString &defaultSaveDir,
                         db::IUserdataDatabase &userdataDatabase,
                         gui::GameImageProvider &gameImageProvider)
    : m_userdataDatabase(userdataDatabase),
      m_gameImageProvider(gameImageProvider) {
  m_ioThreadPool = std::make_unique<QThreadPool>();
  m_ioThreadPool->setMaxThreadCount(1);

  m_settings.beginGroup("Saves");
  m_saveDirectory =
      m_settings.value("SaveDirectory", defaultSaveDir).toString();
}
SaveManager::~SaveManager() {
  m_settings.setValue("SaveDirectory", m_saveDirectory);
}

std::vector<SavefileInfo>
SaveManager::getSaveFileInfoList(const QString &contentHash) const {
  std::vector<SavefileInfo> result;

  for (auto i = 0; i < 8; ++i) {
    auto dir =
        m_saveDirectory + "/" + contentHash + "/slot" + QString::number(i + 1);
    if (!QDir(dir).exists()) {
      result.emplace_back(SavefileInfo{.hasData = false, .slotNumber = i + 1});
      continue;
    }

    const auto saveFile =
        std::filesystem::path((dir + "/savefile.srm").toStdString());

    if (!exists(saveFile)) {
      result.emplace_back(SavefileInfo{.hasData = false, .slotNumber = i + 1});
      continue;
    }

    QFileInfo fileInfo(QString::fromStdString(saveFile.string()));

    SavefileInfo info;
    info.hasData = true;
    info.contentHash = contentHash.toStdString();
    info.filePath = saveFile.string();
    info.slotNumber = i + 1;
    info.lastModifiedEpochSeconds = fileInfo.lastModified().toSecsSinceEpoch();

    result.emplace_back(info);
  }

  return result;
}

std::future<bool> SaveManager::writeSaveData(const QString &contentHash,
                                             int saveSlotNumber,
                                             const Savefile &saveData) {
  return std::async(std::launch::async, [this, contentHash, saveSlotNumber,
                                         saveData] {
    // TODO: Add some verification that the metadata is correct
    // TODO: Save file could have been deleted, etc

    auto exists = false;
    db::SavefileMetadata metadata;

    auto metadataOpt = m_userdataDatabase.getSavefileMetadata(
        contentHash.toStdString(), saveSlotNumber);

    if (metadataOpt.has_value()) {
      // printf("Metadata exists\n");
      metadata = metadataOpt.value();
      exists = true;
    } else {
      // printf("Metadata DOES NOT exist\n");
      metadata.contentId = contentHash.toStdString();
      metadata.slotNumber = saveSlotNumber;
    }

    const auto bytes = saveData.getSaveRamData();

    auto savefileMd5 = QCryptographicHash::hash(bytes, QCryptographicHash::Md5)
                           .toHex()
                           .toStdString();

    if (metadata.savefileMd5 == savefileMd5) {
      return true;
    }

    spdlog::info("Writing updated savefile for {} slot {}",
                 contentHash.toStdString(), saveSlotNumber);
    metadata.savefileMd5 = savefileMd5;

    auto dir = m_saveDirectory + "/" + contentHash + "/slot" +
               QString::number(saveSlotNumber);
    if (!QDir(m_saveDirectory + "/" + contentHash)
             .mkpath("slot" + QString::number(saveSlotNumber))) {
      return false;
    }

    const auto tempSaveFile =
        std::filesystem::path((dir + "/savefile.srm.tmp").toStdString());
    const auto saveFile =
        std::filesystem::path((dir + "/savefile.srm").toStdString());

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
    // also Crystal is cool
    return true;
  });
}

std::optional<Savefile> SaveManager::readSaveData(const QString &contentHash,
                                                  int saveSlotNumber) const {
  auto dir = m_saveDirectory + "/" + contentHash + "/slot" +
             QString::number(saveSlotNumber);
  if (!QDir(dir).exists()) {
    return {};
  }

  const auto saveFile =
      std::filesystem::path((dir + "/savefile.srm").toStdString());

  spdlog::info("[SaveDataService] Reading from save file: {}",
               saveFile.string());

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

QFuture<bool> SaveManager::writeSuspendPoint(const QString &contentHash,
                                             int saveSlotNumber, int index,
                                             const SuspendPoint &suspendPoint) {
  writeSuspendPointToDisk(contentHash, index, suspendPoint);
  emit suspendPointUpdated(contentHash, saveSlotNumber, index);
  return {};
}

std::optional<SuspendPoint>
SaveManager::readSuspendPoint(const QString &contentHash, int saveSlotNumber,
                              int index) {
  return readSuspendPointFromDisk(contentHash, saveSlotNumber, index);
}

void SaveManager::deleteSuspendPoint(const QString &contentHash,
                                     const int saveSlotNumber,
                                     const int index) {
  deleteSuspendPointFromDisk(contentHash, saveSlotNumber, index);
  emit suspendPointDeleted(contentHash, saveSlotNumber, index);
}

QString SaveManager::getSaveDirectory() const { return m_saveDirectory; }

void SaveManager::setSaveDirectory(const QString &saveDirectory) {
  auto temp = saveDirectory;

  if (temp.startsWith("file://")) {
    temp = temp.remove(0, 7);
  }
  if (temp.startsWith("/")) {
    temp = temp.remove(0, 1);
  }

  if (saveDirectory == m_saveDirectory) {
    return;
  }

  m_saveDirectory = temp;
  m_settings.setValue("SaveDirectory", m_saveDirectory);
  emit saveDirectoryChanged(m_saveDirectory);
}

void SaveManager::handleUpdatedSuspendPoint(int index) {
  // auto slotNumber = index + 1;
  // if (m_currentSuspendPointListContentHash.isEmpty() ||
  //     m_currentSuspendPointListSaveSlotNumber == -1) {
  //   return;
  // }
  //
  // auto item = m_suspendPointListModel->getItem(index);
  // if (!item.has_value()) {
  //   return;
  // }
  //
  // if (!item->hasData) {
  //   deleteSuspendPointFromDisk(m_currentSuspendPointListContentHash,
  //                              m_currentSuspendPointListSaveSlotNumber,
  //                              index);
  //   return;
  // }
  //
  // auto metadata = m_userdataDatabase.getSuspendPointMetadata(
  //     m_currentSuspendPointListContentHash.toStdString(),
  //     m_currentSuspendPointListSaveSlotNumber, slotNumber);
  // if (metadata.has_value()) {
  //   metadata->lastModifiedAt = QDateTime::currentMSecsSinceEpoch();
  //   metadata->locked = item->locked;
  //   m_userdataDatabase.updateSuspendPointMetadata(*metadata);
  // } else {
  //   auto ms = QDateTime::currentMSecsSinceEpoch();
  //   db::SuspendPointMetadata newMetadata{
  //       .contentId = m_currentSuspendPointListContentHash.toStdString(),
  //       .saveSlotNumber = m_currentSuspendPointListSaveSlotNumber,
  //       .slotNumber = static_cast<unsigned int>(slotNumber),
  //       .lastModifiedAt = static_cast<uint64_t>(ms),
  //       .createdAt = static_cast<uint64_t>(ms),
  //       .locked = item->locked};
  //
  //   m_userdataDatabase.createSuspendPointMetadata(newMetadata);
  // }
  //
  // metadata->locked = item->locked;
  //
  // m_userdataDatabase.updateSuspendPointMetadata(*metadata);
}

void SaveManager::writeSuspendPointToDisk(const QString &contentHash, int index,
                                          const SuspendPoint &suspendPoint) {
  unsigned int slotNumber = index + 1;
  if (suspendPoint.locked) {
    spdlog::warn("Trying to write locked suspend point for entry with content "
                 "hash {} and index {}",
                 contentHash.toStdString(), index);
    return;
  }

  auto dir = QString("%1/%2/slot%3/suspendpoints/slot%4")
                 .arg(m_saveDirectory, contentHash)
                 .arg(suspendPoint.saveSlotNumber)
                 .arg(slotNumber);

  if (!QDir(dir).exists() && !QDir().mkpath(dir)) {
    return;
  }

  QSaveFile saveFile(dir + "/suspendpoint.state");
  saveFile.open(QIODeviceBase::WriteOnly);

  saveFile.write(QByteArray::fromRawData(
      reinterpret_cast<const char *>(suspendPoint.state.data()),
      suspendPoint.state.size()));
  if (!saveFile.commit()) {
    spdlog::error("Could not write suspend point to disk");
    return;
  }

  if (!suspendPoint.image.isNull()) {
    auto imageFilename = dir + "/screenshot.png";
    if (!suspendPoint.image.save(imageFilename, "PNG")) {
      spdlog::warn("Could not save suspend point image");
    }
  }

  if (!suspendPoint.retroachievementsState.empty()) {
    QSaveFile rcheevosFile(dir + "/rcheevos.state");
    rcheevosFile.open(QIODeviceBase::WriteOnly);
    rcheevosFile.write(
        QByteArray::fromRawData(reinterpret_cast<const char *>(
                                    suspendPoint.retroachievementsState.data()),
                                suspendPoint.retroachievementsState.size()));

    if (!rcheevosFile.commit()) {
      spdlog::error("Could not write rcheevos state to disk");
      return;
    }
  }

  auto metadata = m_userdataDatabase.getSuspendPointMetadata(
      contentHash.toStdString(), suspendPoint.saveSlotNumber, slotNumber);
  if (metadata.has_value()) {
    metadata->lastModifiedAt = QDateTime::currentMSecsSinceEpoch();
    metadata->locked = suspendPoint.locked;
    m_userdataDatabase.updateSuspendPointMetadata(*metadata);
  } else {
    auto ms = QDateTime::currentMSecsSinceEpoch();
    db::SuspendPointMetadata newMetadata{
        .contentId = contentHash.toStdString(),
        .saveSlotNumber = suspendPoint.saveSlotNumber,
        .slotNumber = slotNumber,
        .lastModifiedAt = static_cast<uint64_t>(ms),
        .createdAt = static_cast<uint64_t>(ms),
        .locked = suspendPoint.locked};

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

std::optional<SuspendPoint>
SaveManager::readSuspendPointFromDisk(const QString &contentHash,
                                      int saveSlotNumber, int index) const {
  auto slotNumber = index + 1;

  // TODO: metadata

  auto dir = QString("%1/%2/slot%3/suspendpoints/slot%4")
                 .arg(m_saveDirectory, contentHash)
                 .arg(saveSlotNumber)
                 .arg(slotNumber);

  if (!QDir(dir).exists() && !QDir().mkpath(dir)) {
    return {};
  }

  long long created = 0;

  QFile stateFile(dir + "/suspendpoint.state");
  if (!stateFile.open(QIODeviceBase::ReadOnly)) {
    return std::nullopt;
  }

  created =
      stateFile.fileTime(QFileDevice::FileModificationTime).toMSecsSinceEpoch();

  std::vector<uint8_t> data(stateFile.size());
  stateFile.read(reinterpret_cast<char *>(data.data()), stateFile.size());
  stateFile.close();

  QImage image;
  if (QFile(dir + "/screenshot.png").exists()) {
    image.load(dir + "/screenshot.png");
  }

  std::vector<uint8_t> rcheevosData;
  if (auto rcheevosFile = dir + "/rcheevos.state";
      QFile(rcheevosFile).exists()) {
    QFile file(rcheevosFile);
    file.open(QIODeviceBase::ReadOnly);
    auto bytes = file.readAll();
    rcheevosData = std::vector<uint8_t>(bytes.begin(), bytes.end());
    file.close();
  }

  // long long created = 0;
  bool locked = false;

  auto metadata = m_userdataDatabase.getSuspendPointMetadata(
      contentHash.toStdString(), saveSlotNumber, slotNumber);
  if (metadata.has_value()) {
    // created = metadata->createdAt;
    locked = metadata->locked;
  }

  return SuspendPoint{
      .state = data,
      .retroachievementsState = rcheevosData,
      .timestamp = created,
      .image = image,
      .locked = locked,
  };
}

void SaveManager::deleteSuspendPointFromDisk(const QString &contentHash,
                                             int saveSlotNumber, int index) {
  auto slotNumber = index + 1;

  auto dir = QString("%1/%2/slot%3/suspendpoints/slot%4")
                 .arg(m_saveDirectory, contentHash)
                 .arg(saveSlotNumber)
                 .arg(slotNumber);

  if (!QDir(dir).exists()) {
    return;
  }

  auto metadata = m_userdataDatabase.getSuspendPointMetadata(
      contentHash.toStdString(), saveSlotNumber, slotNumber);
  if (metadata.has_value()) {
    m_userdataDatabase.deleteSuspendPointMetadata(metadata->id);
  }

  // TODO: Delete metadata
  spdlog::debug("Deleting suspend point from disk");

  QFile stateFile(dir + "/suspendpoint.state");
  if (stateFile.exists()) {
    stateFile.remove();
  }

  QFile imageFile(dir + "/screenshot.png");
  if (imageFile.exists()) {
    imageFile.remove();
  }

  QFile rcheevosFile(dir + "/rcheevos.state");
  if (rcheevosFile.exists()) {
    rcheevosFile.remove();
  }
}
} // namespace firelight::saves

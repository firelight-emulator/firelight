#include <firelight/saves/save_manager_impl.hpp>

#include <firelight/event_dispatcher.hpp>
#include <firelight/saves/save_events.hpp>

#include <filesystem>
#include <fstream>
#include <future>
#include <qcryptographichash.h>
#include <qdir.h>
#include <qdatetime.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qsavefile.h>
#include <spdlog/spdlog.h>

namespace firelight::saves {
SaveManager::SaveManager(const std::string &defaultSaveDir,
                         db::IUserdataDatabase &userdataDatabase)
    : m_userdataDatabase(userdataDatabase) {
  m_settings.beginGroup("Saves");
  m_saveDirectory =
      m_settings
          .value("SaveDirectory", QString::fromStdString(defaultSaveDir))
          .toString();
}

SaveManager::~SaveManager() {
  m_settings.setValue("SaveDirectory", m_saveDirectory);
}

std::vector<SavefileInfo>
SaveManager::getSaveFileInfoList(const std::string &contentHash) const {
  std::vector<SavefileInfo> result;
  const auto contentHashQ = QString::fromStdString(contentHash);

  for (auto i = 0; i < 8; ++i) {
    auto dir =
        m_saveDirectory + "/" + contentHashQ + "/slot" + QString::number(i + 1);
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
    info.contentHash = contentHash;
    info.filePath = saveFile.string();
    info.slotNumber = i + 1;
    info.lastModifiedEpochSeconds = fileInfo.lastModified().toSecsSinceEpoch();

    result.emplace_back(info);
  }

  return result;
}

std::future<bool> SaveManager::writeSaveData(const std::string &contentHash,
                                             int saveSlotNumber,
                                             const Savefile &saveData) {
  return std::async(std::launch::async, [this, contentHash, saveSlotNumber,
                                         saveData] {
    const auto contentHashQ = QString::fromStdString(contentHash);
    auto exists = false;
    db::SavefileMetadata metadata;

    auto metadataOpt =
        m_userdataDatabase.getSavefileMetadata(contentHash, saveSlotNumber);

    if (metadataOpt.has_value()) {
      metadata = metadataOpt.value();
      exists = true;
    } else {
      metadata.contentId = contentHash;
      metadata.slotNumber = saveSlotNumber;
    }

    const auto bytes = saveData.getSaveRamData();

    auto savefileMd5 = QCryptographicHash::hash(bytes, QCryptographicHash::Md5)
                           .toHex()
                           .toStdString();

    if (metadata.savefileMd5 == savefileMd5) {
      return true;
    }

    spdlog::info("Writing updated savefile for {} slot {}", contentHash,
                 saveSlotNumber);
    metadata.savefileMd5 = savefileMd5;

    auto dir = m_saveDirectory + "/" + contentHashQ + "/slot" +
               QString::number(saveSlotNumber);
    if (!QDir(m_saveDirectory + "/" + contentHashQ)
             .mkpath("slot" + QString::number(saveSlotNumber))) {
      spdlog::error("Failed to create save directory for {} slot {}",
                    contentHash, saveSlotNumber);
      return false;
    }

    const auto tempSaveFile =
        std::filesystem::path((dir + "/savefile.srm.tmp").toStdString());
    const auto saveFile =
        std::filesystem::path((dir + "/savefile.srm").toStdString());

    // Write to a temp file first and verify the write fully succeeded before
    // replacing the existing save, so a failed/partial write cannot corrupt a
    // known-good save.
    {
      std::ofstream saveFileStream(tempSaveFile, std::ios::binary);
      saveFileStream.write(bytes.data(), bytes.size());
      saveFileStream.close();
      if (saveFileStream.fail()) {
        spdlog::error("Failed to write savefile for {} slot {}", contentHash,
                      saveSlotNumber);
        std::error_code ec;
        std::filesystem::remove(tempSaveFile, ec);
        return false;
      }
    }

    std::error_code renameEc;
    std::filesystem::rename(tempSaveFile, saveFile, renameEc);
    if (renameEc) {
      spdlog::error("Failed to replace savefile for {} slot {}: {}", contentHash,
                    saveSlotNumber, renameEc.message());
      std::error_code removeEc;
      std::filesystem::remove(tempSaveFile, removeEc);
      return false;
    }

    metadata.lastModifiedAt = QDateTime::currentSecsSinceEpoch();

    const bool metadataOk =
        exists ? m_userdataDatabase.updateSavefileMetadata(metadata)
               : m_userdataDatabase.createSavefileMetadata(metadata);
    if (!metadataOk) {
      // The save data is safely on disk; only the metadata index failed.
      spdlog::warn(
          "Savefile written but metadata persistence failed for {} slot {}",
          contentHash, saveSlotNumber);
    }

    return true;
  });
}

std::optional<Savefile> SaveManager::readSaveData(const std::string &contentHash,
                                                  int saveSlotNumber) const {
  auto dir = m_saveDirectory + "/" + QString::fromStdString(contentHash) +
             "/slot" + QString::number(saveSlotNumber);
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

void SaveManager::writeSuspendPoint(const std::string &contentHash,
                                    int saveSlotNumber, int index,
                                    const SuspendPoint &suspendPoint) {
  writeSuspendPointToDisk(QString::fromStdString(contentHash), index,
                          suspendPoint);
  EventDispatcher::instance().publish(
      SuspendPointUpdatedEvent{contentHash, saveSlotNumber, index});
}

std::optional<SuspendPoint>
SaveManager::readSuspendPoint(const std::string &contentHash, int saveSlotNumber,
                              int index) {
  return readSuspendPointFromDisk(QString::fromStdString(contentHash),
                                  saveSlotNumber, index);
}

void SaveManager::deleteSuspendPoint(const std::string &contentHash,
                                     const int saveSlotNumber,
                                     const int index) {
  deleteSuspendPointFromDisk(QString::fromStdString(contentHash), saveSlotNumber,
                             index);
  EventDispatcher::instance().publish(
      SuspendPointDeletedEvent{contentHash, saveSlotNumber, index});
}

int SaveManager::copySavefilesForward(const std::string &fromContentHash,
                                      const std::string &toContentHash) {
  if (fromContentHash == toContentHash || fromContentHash.empty() ||
      toContentHash.empty()) {
    return 0;
  }

  const auto fromHashQ = QString::fromStdString(fromContentHash);
  const auto toHashQ = QString::fromStdString(toContentHash);

  int copied = 0;
  // Mirror getSaveFileInfoList's 8-slot layout: slot1..slot8.
  for (auto i = 0; i < 8; ++i) {
    const auto slotNumber = i + 1;
    const auto slot = "/slot" + QString::number(slotNumber);

    const auto sourceFile = std::filesystem::path(
        (m_saveDirectory + "/" + fromHashQ + slot + "/savefile.srm")
            .toStdString());
    if (!std::filesystem::exists(sourceFile)) {
      continue;
    }

    const auto destDirQ = m_saveDirectory + "/" + toHashQ + slot;
    const auto destFile =
        std::filesystem::path((destDirQ + "/savefile.srm").toStdString());
    // Never overwrite an existing save at the destination.
    if (std::filesystem::exists(destFile)) {
      continue;
    }

    if (!QDir(m_saveDirectory + "/" + toHashQ)
             .mkpath("slot" + QString::number(slotNumber))) {
      spdlog::error("Failed to create save directory for {} slot {}",
                    toContentHash, slotNumber);
      continue;
    }

    // Copy through a temp file + rename so an interrupted copy cannot leave a
    // partial save at the destination.
    const auto tempFile =
        std::filesystem::path((destDirQ + "/savefile.srm.tmp").toStdString());
    std::error_code ec;
    std::filesystem::copy_file(
        sourceFile, tempFile,
        std::filesystem::copy_options::overwrite_existing, ec);
    if (ec) {
      spdlog::error("Failed to copy savefile forward for {} slot {}: {}",
                    toContentHash, slotNumber, ec.message());
      std::filesystem::remove(tempFile, ec);
      continue;
    }
    std::filesystem::rename(tempFile, destFile, ec);
    if (ec) {
      spdlog::error("Failed to finalize copied savefile for {} slot {}: {}",
                    toContentHash, slotNumber, ec.message());
      std::filesystem::remove(tempFile, ec);
      continue;
    }

    // Index the copied save so it is visible without a re-scan.
    if (!m_userdataDatabase.getSavefileMetadata(toContentHash, slotNumber)
             .has_value()) {
      QFile copiedFile(QString::fromStdString(destFile.string()));
      std::string md5;
      if (copiedFile.open(QIODeviceBase::ReadOnly)) {
        md5 = QCryptographicHash::hash(copiedFile.readAll(),
                                       QCryptographicHash::Md5)
                  .toHex()
                  .toStdString();
      }
      db::SavefileMetadata metadata;
      metadata.contentId = toContentHash;
      metadata.slotNumber = slotNumber;
      metadata.savefileMd5 = md5;
      metadata.lastModifiedAt = QDateTime::currentSecsSinceEpoch();
      m_userdataDatabase.createSavefileMetadata(metadata);
    }

    ++copied;
  }

  return copied;
}

std::string SaveManager::getSaveDirectory() const {
  return m_saveDirectory.toStdString();
}

void SaveManager::setSaveDirectory(const std::string &saveDirectory) {
  const auto raw = QString::fromStdString(saveDirectory);
  auto temp = raw;

  if (temp.startsWith("file://")) {
    temp = temp.remove(0, 7);
  }
  if (temp.startsWith("/")) {
    temp = temp.remove(0, 1);
  }

  if (raw == m_saveDirectory) {
    return;
  }

  m_saveDirectory = temp;
  m_settings.setValue("SaveDirectory", m_saveDirectory);
}

void SaveManager::writeSuspendPointToDisk(const QString &contentHash,
                                          int index,
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
    QSaveFile imageFile(dir + "/screenshot.png");
    imageFile.open(QIODeviceBase::WriteOnly);
    imageFile.write(
        reinterpret_cast<const char *>(suspendPoint.image.pngData.data()),
        static_cast<qint64>(suspendPoint.image.pngData.size()));
    if (!imageFile.commit()) {
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
}

std::optional<SuspendPoint>
SaveManager::readSuspendPointFromDisk(const QString &contentHash,
                                      int saveSlotNumber, int index) const {
  auto slotNumber = index + 1;

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

  firelight::Image image;
  if (QFile imageFile(dir + "/screenshot.png"); imageFile.exists()) {
    if (imageFile.open(QIODeviceBase::ReadOnly)) {
      const auto bytes = imageFile.readAll();
      image.pngData.assign(bytes.begin(), bytes.end());
    }
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

  bool locked = false;

  auto metadata = m_userdataDatabase.getSuspendPointMetadata(
      contentHash.toStdString(), saveSlotNumber, slotNumber);
  if (metadata.has_value()) {
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

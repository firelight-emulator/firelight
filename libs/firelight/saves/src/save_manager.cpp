#include <firelight/saves/save_manager_impl.hpp>

#include <firelight/event_dispatcher.hpp>
#include <firelight/saves/save_events.hpp>

#include "firelight/saves/detail/md5.hpp"

#include <spdlog/spdlog.h>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <future>
#include <system_error>

namespace fs = std::filesystem;

namespace firelight::saves {
namespace {
int64_t nowMs() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch())
      .count();
}

// Last-modified time of a file as int64 milliseconds since the Unix epoch
// Portable conversion from filesystem clock to system clock (C++20 clock_cast
// support is uneven across the toolchains we build on)
int64_t fileMtimeMs(const fs::path &p) {
  std::error_code ec;
  const auto ftime = fs::last_write_time(p, ec);
  if (ec)
    return 0;
  using namespace std::chrono;
  const auto sctp = time_point_cast<system_clock::duration>(
      ftime - fs::file_time_type::clock::now() + system_clock::now());
  return duration_cast<milliseconds>(sctp.time_since_epoch()).count();
}

// Atomically replace `path` with `data` via a temp file + rename, so a failed
// or partial write can never corrupt a prior good file. Returns false on error
bool writeAtomic(const fs::path &path, const void *data, size_t size) {
  const fs::path tmp = path.string() + ".tmp";
  {
    std::ofstream out(tmp, std::ios::binary);
    out.write(static_cast<const char *>(data), static_cast<std::streamsize>(size));
    out.close();
    if (out.fail()) {
      std::error_code ec;
      fs::remove(tmp, ec);
      return false;
    }
  }
  std::error_code ec;
  fs::rename(tmp, path, ec);
  if (ec) {
    std::error_code rmEc;
    fs::remove(tmp, rmEc);
    return false;
  }
  return true;
}

// Convert a possibly-file:// URL into a filesystem path. Strips the scheme but,
// unlike the old code, does NOT strip the leading '/' of a POSIX absolute path
// (that turned /Users/x into Users/x). Only a Windows drive path (/C:/...) has
// its leading slash removed
std::string stripFileUrl(std::string s) {
  constexpr std::string_view SCHEME = "file://";
  if (s.rfind(SCHEME, 0) == 0)
    s.erase(0, SCHEME.size());
  if (s.size() >= 3 && s[0] == '/' &&
      ((s[1] >= 'A' && s[1] <= 'Z') || (s[1] >= 'a' && s[1] <= 'z')) &&
      s[2] == ':')
    s.erase(0, 1);
  return s;
}

std::string slotDir(const std::string &root, const std::string &contentHash,
                    int slot) {
  return root + "/" + contentHash + "/slot" + std::to_string(slot);
}

std::string suspendDir(const std::string &root, const std::string &contentHash,
                       int saveSlot, unsigned int slotNumber) {
  return root + "/" + contentHash + "/slot" + std::to_string(saveSlot) +
         "/suspendpoints/slot" + std::to_string(slotNumber);
}
} // namespace

SaveManager::SaveManager(const std::string &saveDir, ISaveDatabase &saveDatabase)
    : m_saveDatabase(saveDatabase), m_saveDirectory(saveDir) {}

SaveManager::~SaveManager() = default;

std::vector<SavefileInfo>
SaveManager::getSaveFileInfoList(const std::string &contentHash) const {
  std::vector<SavefileInfo> result;

  for (auto i = 0; i < 8; ++i) {
    const fs::path saveFile =
        fs::path(slotDir(m_saveDirectory, contentHash, i + 1)) / "savefile.srm";

    if (std::error_code ec; !fs::exists(saveFile, ec)) {
      result.emplace_back(SavefileInfo{.hasData = false, .slotNumber = i + 1});
      continue;
    }

    SavefileInfo info;
    info.hasData = true;
    info.contentHash = contentHash;
    info.filePath = saveFile.string();
    info.slotNumber = i + 1;
    info.lastModifiedEpochSeconds = fileMtimeMs(saveFile) / 1000;

    result.emplace_back(info);
  }

  return result;
}

std::future<bool> SaveManager::writeSaveData(const std::string &contentHash,
                                             int saveSlotNumber,
                                             const Savefile &saveData) {
  return std::async(std::launch::async, [this, contentHash, saveSlotNumber,
                                         saveData] {
    auto exists = false;
    SavefileMetadata metadata;

    auto metadataOpt =
        m_saveDatabase.getSavefileMetadata(contentHash, saveSlotNumber);

    if (metadataOpt.has_value()) {
      metadata = metadataOpt.value();
      exists = true;
    } else {
      metadata.contentId = contentHash;
      metadata.slotNumber = saveSlotNumber;
    }

    const auto bytes = saveData.getSaveRamData();

    const auto savefileMd5 = detail::Md5::hash(bytes.data(), bytes.size());

    if (metadata.savefileMd5 == savefileMd5) {
      return true;
    }

    spdlog::info("Writing updated savefile for {} slot {}", contentHash,
                 saveSlotNumber);
    metadata.savefileMd5 = savefileMd5;

    const auto dir = slotDir(m_saveDirectory, contentHash, saveSlotNumber);
    std::error_code mkEc;
    fs::create_directories(dir, mkEc);
    if (mkEc) {
      spdlog::error("Failed to create save directory for {} slot {}: {}",
                    contentHash, saveSlotNumber, mkEc.message());
      return false;
    }

    const fs::path saveFile = fs::path(dir) / "savefile.srm";
    if (!writeAtomic(saveFile, bytes.data(), bytes.size())) {
      spdlog::error("Failed to write savefile for {} slot {}", contentHash,
                    saveSlotNumber);
      return false;
    }

    metadata.lastModifiedAt = nowMs();

    const bool metadataOk =
        exists ? m_saveDatabase.updateSavefileMetadata(metadata)
               : m_saveDatabase.createSavefileMetadata(metadata);
    if (!metadataOk) {
      // The save data is safely on disk; only the metadata index failed
      spdlog::warn(
          "Savefile written but metadata persistence failed for {} slot {}",
          contentHash, saveSlotNumber);
    }

    return true;
  });
}

std::optional<Savefile> SaveManager::readSaveData(const std::string &contentHash,
                                                  int saveSlotNumber) const {
  const fs::path saveFile =
      fs::path(slotDir(m_saveDirectory, contentHash, saveSlotNumber)) /
      "savefile.srm";

  if (std::error_code ec; !fs::exists(saveFile, ec)) {
    return std::nullopt;
  }

  spdlog::info("[SaveDataService] Reading from save file: {}",
               saveFile.string());

  std::ifstream saveFileStream(saveFile, std::ios::binary);
  const auto size = fs::file_size(saveFile);
  std::vector<char> data(size);
  saveFileStream.read(data.data(), static_cast<std::streamsize>(size));
  saveFileStream.close();

  return Savefile(data);
}

void SaveManager::writeSuspendPoint(const std::string &contentHash,
                                    int saveSlotNumber, int index,
                                    const SuspendPoint &suspendPoint) {
  writeSuspendPointToDisk(contentHash, index, suspendPoint);
  EventDispatcher::instance().publish(
      SuspendPointUpdatedEvent{contentHash, saveSlotNumber, index});
}

std::optional<SuspendPoint>
SaveManager::readSuspendPoint(const std::string &contentHash, int saveSlotNumber,
                              int index) {
  return readSuspendPointFromDisk(contentHash, saveSlotNumber, index);
}

void SaveManager::deleteSuspendPoint(const std::string &contentHash,
                                     const int saveSlotNumber,
                                     const int index) {
  deleteSuspendPointFromDisk(contentHash, saveSlotNumber, index);
  EventDispatcher::instance().publish(
      SuspendPointDeletedEvent{contentHash, saveSlotNumber, index});
}

std::string SaveManager::getSaveDirectory() const { return m_saveDirectory; }

void SaveManager::setSaveDirectory(const std::string &saveDirectory) {
  const auto stripped = stripFileUrl(saveDirectory);
  if (stripped == m_saveDirectory) {
    return;
  }
  m_saveDirectory = stripped;
}

void SaveManager::writeSuspendPointToDisk(const std::string &contentHash,
                                          int index,
                                          const SuspendPoint &suspendPoint) {
  const unsigned int slotNumber = index + 1;
  if (suspendPoint.locked) {
    spdlog::warn("Trying to write locked suspend point for entry with content "
                 "hash {} and index {}",
                 contentHash, index);
    return;
  }

  const auto dir = suspendDir(m_saveDirectory, contentHash,
                              suspendPoint.saveSlotNumber, slotNumber);
  std::error_code mkEc;
  fs::create_directories(dir, mkEc);
  if (mkEc) {
    return;
  }

  if (!writeAtomic(fs::path(dir) / "suspendpoint.state",
                   suspendPoint.state.data(), suspendPoint.state.size())) {
    spdlog::error("Could not write suspend point to disk");
    return;
  }

  if (!suspendPoint.image.isNull()) {
    if (!writeAtomic(fs::path(dir) / "screenshot.png",
                     suspendPoint.image.pngData.data(),
                     suspendPoint.image.pngData.size())) {
      spdlog::warn("Could not save suspend point image");
    }
  }

  if (!suspendPoint.retroachievementsState.empty()) {
    if (!writeAtomic(fs::path(dir) / "rcheevos.state",
                     suspendPoint.retroachievementsState.data(),
                     suspendPoint.retroachievementsState.size())) {
      spdlog::error("Could not write rcheevos state to disk");
      return;
    }
  }

  auto metadata = m_saveDatabase.getSuspendPointMetadata(
      contentHash, suspendPoint.saveSlotNumber, slotNumber);
  if (metadata.has_value()) {
    metadata->lastModifiedAt = nowMs();
    metadata->locked = suspendPoint.locked;
    m_saveDatabase.updateSuspendPointMetadata(*metadata);
  } else {
    const auto ms = nowMs();
    SuspendPointMetadata newMetadata{
        .contentId = contentHash,
        .saveSlotNumber = suspendPoint.saveSlotNumber,
        .slotNumber = slotNumber,
        .lastModifiedAt = ms,
        .createdAt = ms,
        .locked = suspendPoint.locked};

    m_saveDatabase.createSuspendPointMetadata(newMetadata);
  }
}

std::optional<SuspendPoint>
SaveManager::readSuspendPointFromDisk(const std::string &contentHash,
                                      int saveSlotNumber, int index) const {
  const unsigned int slotNumber = index + 1;
  const auto dir =
      suspendDir(m_saveDirectory, contentHash, saveSlotNumber, slotNumber);

  const fs::path stateFile = fs::path(dir) / "suspendpoint.state";
  if (std::error_code ec; !fs::exists(stateFile, ec)) {
    return std::nullopt;
  }

  const int64_t created = fileMtimeMs(stateFile);

  std::vector<uint8_t> data;
  {
    std::ifstream in(stateFile, std::ios::binary);
    const auto size = fs::file_size(stateFile);
    data.resize(size);
    in.read(reinterpret_cast<char *>(data.data()),
            static_cast<std::streamsize>(size));
  }

  firelight::Image image;
  if (const fs::path imagePath = fs::path(dir) / "screenshot.png";
      fs::exists(imagePath)) {
    std::ifstream in(imagePath, std::ios::binary);
    const auto size = fs::file_size(imagePath);
    image.pngData.resize(size);
    in.read(reinterpret_cast<char *>(image.pngData.data()),
            static_cast<std::streamsize>(size));
  }

  std::vector<uint8_t> rcheevosData;
  if (const fs::path rcheevosPath = fs::path(dir) / "rcheevos.state";
      fs::exists(rcheevosPath)) {
    std::ifstream in(rcheevosPath, std::ios::binary);
    const auto size = fs::file_size(rcheevosPath);
    rcheevosData.resize(size);
    in.read(reinterpret_cast<char *>(rcheevosData.data()),
            static_cast<std::streamsize>(size));
  }

  bool locked = false;
  auto metadata = m_saveDatabase.getSuspendPointMetadata(contentHash,
                                                         saveSlotNumber, slotNumber);
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

void SaveManager::deleteSuspendPointFromDisk(const std::string &contentHash,
                                             int saveSlotNumber, int index) {
  const unsigned int slotNumber = index + 1;
  const auto dir =
      suspendDir(m_saveDirectory, contentHash, saveSlotNumber, slotNumber);

  if (std::error_code ec; !fs::exists(dir, ec)) {
    return;
  }

  auto metadata = m_saveDatabase.getSuspendPointMetadata(contentHash,
                                                         saveSlotNumber, slotNumber);
  if (metadata.has_value()) {
    m_saveDatabase.deleteSuspendPointMetadata(metadata->id);
  }

  spdlog::debug("Deleting suspend point from disk");

  std::error_code ec;
  fs::remove(fs::path(dir) / "suspendpoint.state", ec);
  fs::remove(fs::path(dir) / "screenshot.png", ec);
  fs::remove(fs::path(dir) / "rcheevos.state", ec);
}
} // namespace firelight::saves

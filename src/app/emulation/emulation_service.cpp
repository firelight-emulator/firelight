#include "emulation_service.hpp"

#include "event_dispatcher.hpp"
#include "input2/input_service.hpp"
#include "platforms/platform_service.hpp"

#include <audio/audio_manager.hpp>
#include <library/rom_file.hpp>
#include <libretro/core.hpp>
#include <platform_metadata.hpp>
#include <spdlog/spdlog.h>

firelight::emulation::EmulationService
    *firelight::emulation::EmulationService::s_emuServiceInstance = nullptr;

namespace firelight::emulation {
EmulationService::EmulationService(library::IUserLibrary &library)
    : m_library(library) {}
EmulationService::~EmulationService() {
  spdlog::info("[EmulationService] Stopping EmulationService");
}
std::future<EmulatorInstance *> EmulationService::loadEntry(int entryId) {
  if (m_emulatorInstance) {
    stopEmulation();
  }

  spdlog::info("[EmulationService] Loading entry with id {}", entryId);

  auto entry = m_library.getEntry(entryId);

  if (!entry.has_value()) {
    spdlog::warn("[EmulationService] Entry with id {} does not exist", entryId);
    EventDispatcher::instance().publish(GameLoadFailedEvent{});

    std::promise<EmulatorInstance *> promise;
    promise.set_value(nullptr);
    return promise.get_future();
  }

  auto runConfigurations = m_library.getRunConfigurations(entry->contentHash);

  if (runConfigurations.empty()) {
    spdlog::warn("[EmulationService] No run configuration found for "
                 "entry with id {}",
                 entryId);
    return {};
  }

  auto runConfig = runConfigurations[0];
  spdlog::info("[EmulationService] Got run configuration for entry with "
               "contentHash {}: {}",
               entry->contentHash.toStdString(), runConfig.id);

  if (runConfig.type == library::RunConfiguration::TYPE_ROM) {
    auto romId = runConfig.romId;
    auto romInfo = m_library.getRomFile(romId);

    if (!romInfo.has_value()) {
      return {};
    }

    auto filePath = QString::fromStdString(romInfo->m_filePath);
    if (romInfo->m_inArchive) {
      filePath = QString::fromStdString(romInfo->m_archivePathName);
    }

    auto file = QFile(filePath);
    if (!file.exists()) {
      spdlog::error("[EmulationService] Content path doesn't exist: {}",
                    filePath.toStdString());
      return {};
    }

    file.open(QIODevice::ReadOnly);
    auto bytes = file.readAll();
    file.close();

    auto rom = library::RomFile(
        QString::fromStdString(romInfo->m_filePath), bytes.data(), bytes.size(),
        QString::fromStdString(romInfo->m_archivePathName));

    if (rom.inArchive() &&
        !std::filesystem::exists(rom.getArchivePathName().toStdString())) {
      spdlog::error("[EmulationService] Content path doesn't exist: {}",
                    rom.getArchivePathName().toStdString());
      return {};
    }

    if (!rom.inArchive() &&
        !std::filesystem::exists(rom.getFilePath().toStdString())) {
      spdlog::error("[EmulationService] Content path doesn't exist: {}",
                    rom.getFilePath().toStdString());
      return {};
    }

    rom.load();

    std::string corePath = PlatformMetadata::getCoreDllPath(entry->platformId);

    QByteArray saveDataBytes;
    const auto saveData = getSaveManager()->readSaveData(rom.getContentHash(),
                                                         entry->activeSaveSlot);
    if (saveData.has_value()) {
      saveDataBytes = QByteArray(saveData->getSaveRamData().data(),
                                 saveData->getSaveRamData().size());
    }

    m_currentEntry = entry.value();
    m_currentContentHash = m_currentEntry.contentHash.toStdString();

    auto platform = platforms::PlatformService::getInstance().getPlatform(
        m_currentEntry.platformId);
    if (platform) {
      m_currentPlatform = platform.value();
    }
    auto configProvider = getEmulatorConfigManager()->getCoreConfigFor(
        m_currentPlatform.id, m_currentEntry.contentHash);
    auto m_core = std::make_unique<::libretro::Core>(m_currentEntry.platformId,
                                                     corePath, configProvider,
                                                     getCoreSystemDirectory());

    auto contentBytes = rom.getContentBytes();

    m_emulatorInstance = std::make_unique<EmulatorInstance>(
        std::move(m_core), rom.getFilePath().toStdString(),
        m_currentContentHash, entry->platformId, entry->activeSaveSlot,
        std::vector<uint8_t>(contentBytes.begin(), contentBytes.end()),
        std::vector<uint8_t>(saveDataBytes.begin(), saveDataBytes.end()));

    EventDispatcher::instance().publish(GameLoadedEvent{});
    // return m_emulatorInstance.get();
  }

  // if (runConfig.type == library::RunConfiguration::TYPE_PATCH) {
  //   auto romId = runConfig.romId;
  //   auto romInfo = m_library.getRomFile(romId);
  //
  //   if (!romInfo.has_value()) {
  //     return {};
  //   }
  //
  //   auto rom = library::RomFile(QString::fromStdString(romInfo->m_filePath));
  //
  //   rom.load();
  //
  //   if (!rom.isValid()) {
  //     return {};
  //   }
  //
  //   auto patchId = runConfig.patchId;
  //   auto patch = m_library.getPatchFile(patchId);
  //
  //   if (!patch.has_value()) {
  //     return {};
  //   }
  //
  //   patch->load();
  //   rom.applyPatchToContentBytes(*patch);
  //
  //   std::string corePath =
  //   PlatformMetadata::getCoreDllPath(entry->platformId);
  //
  //   QByteArray saveDataBytes;
  //   const auto saveData =
  //   getSaveManager()->readSaveData(rom.getContentHash(),
  //                                                        entry->activeSaveSlot);
  //   if (saveData.has_value()) {
  //     saveDataBytes = QByteArray(saveData->getSaveRamData().data(),
  //                                saveData->getSaveRamData().size());
  //   }
  //
  //   return {};
  // }

  return std::async(std::launch::async, [entryId, this] -> EmulatorInstance * {
    return m_emulatorInstance.get();
  });
}
void EmulationService::stopEmulation() {
  m_emulatorInstance.reset();
  EventDispatcher::instance().publish(EmulationStoppedEvent{});
}
EmulatorInstance *EmulationService::getCurrentEmulatorInstance() {
  return m_emulatorInstance.get();
}

bool EmulationService::isGameRunning() const {
  return m_emulatorInstance != nullptr;
}

std::optional<std::string> EmulationService::getCurrentGameName() const {
  return isGameRunning() ? m_currentEntry.displayName.toStdString() : "";
}
std::optional<library::Entry> EmulationService::getCurrentEntry() {
  return isGameRunning() ? std::optional(m_currentEntry) : std::nullopt;
}

std::optional<platforms::Platform>
EmulationService::getCurrentPlatform() const {
  return {};
}
} // namespace firelight::emulation

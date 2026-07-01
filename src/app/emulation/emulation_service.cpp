#include "emulation_service.hpp"

#include <qfile.h>
#include "firelight/event_dispatcher.hpp"
#include <firelight/input/input_service.hpp>
#include <firelight/platforms/platform_service.hpp>

#include <audio/audio_manager.hpp>
#include <firelight/library/content_loader.hpp>
#include <firelight/library/entry_resolver.hpp>
#include <firelight/library/user_library_service.hpp>
#include <libretro/core.hpp>
#include <platform_metadata.hpp>
#include <spdlog/spdlog.h>

firelight::emulation::EmulationService
*firelight::emulation::EmulationService::s_emuServiceInstance = nullptr;

namespace firelight::emulation {
  EmulationService::EmulationService(library::UserLibraryService &library,
                                     library::EntryResolver &entryResolver,
                                     settings::SettingsService &settingsService)
    : m_settingsService(settingsService), m_library(library),
      m_resolver(entryResolver) {
  }

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

    // Pick the most correct content (ROM/disc + optional patch) for this entry.
    const auto resolved = m_resolver.resolve(*entry);
    if (!resolved.valid) {
      spdlog::warn("[EmulationService] No usable content for entry with id {}",
                   entryId);
      return {};
    }

    const auto &contentFile = resolved.contentFile;

    const auto contentPath =
        contentFile.m_inArchive ? contentFile.m_archivePathName : contentFile.m_filePath;
    if (!std::filesystem::exists(contentPath)) {
      spdlog::error("[EmulationService] Content path doesn't exist: {}",
                    contentPath);
      return {};
    }

    library::ContentLoader contentLoader;
    auto loaded = contentLoader.load(contentFile);
    if (!loaded.valid) {
      spdlog::error("[EmulationService] Failed to load content for entry {}",
                    entryId);
      return {};
    }

    if (resolved.patch.has_value()) {
      auto patch = *resolved.patch;
      if (patch.load()) {
        contentLoader.applyPatch(loaded, contentFile.m_platformId, patch);
      } else {
        spdlog::error("[EmulationService] Failed to load patch {} for entry {}",
                      patch.m_filePath, entryId);
      }
    }

    std::string corePath = PlatformMetadata::getCoreDllPath(entry->platformId);

    QByteArray saveDataBytes;
    const auto saveData = getSaveManager()->readSaveData(
        QString::fromStdString(loaded.contentHash), entry->activeSaveSlot);
    if (saveData.has_value()) {
      saveDataBytes = QByteArray(saveData->getSaveRamData().data(),
                                 saveData->getSaveRamData().size());
    }

    m_currentEntry = entry.value();
    m_currentContentHash = m_currentEntry.contentHash.toStdString();

    if (auto platform = platforms::PlatformService::getInstance().getPlatform(
      m_currentEntry.platformId)) {
      m_currentPlatform = platform.value();
    }

    auto coreConfig = std::make_shared<CoreConfiguration>(
      m_currentEntry.contentHash.toStdString(), m_currentPlatform,
      m_settingsService);

    auto m_core = std::make_unique<::libretro::Core>(m_currentEntry.platformId,
                                                     corePath, coreConfig,
                                                     getCoreSystemDirectory());

    m_emulatorInstance = std::make_unique<EmulatorInstance>(
      std::move(m_core), contentFile.m_filePath, m_currentContentHash,
      entry->platformId, entry->activeSaveSlot, std::move(loaded.contentBytes),
      std::vector<uint8_t>(saveDataBytes.begin(), saveDataBytes.end()));

    EventDispatcher::instance().publish(GameLoadedEvent{});

    return std::async(std::launch::async, [this]() -> EmulatorInstance *{
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
    return m_currentPlatform;
  }
} // namespace firelight::emulation

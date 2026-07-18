#include "game_loader.hpp"

#include <firelight/saves/isave_manager.hpp>
#include <qfile.h>

#include <firelight/library/content_loader.hpp>
#include <firelight/library/entry_resolver.hpp>
#include <firelight/library/user_library_service.hpp>
#include <firelight/platforms/platform_service.hpp>
#include <firelight/settings/settings_catalog.hpp>
#include <libretro/core_configuration.hpp>
#include <libretro/core_registry.hpp>
#include <spdlog/spdlog.h>

#include <filesystem>

namespace firelight::emulation {
  GameLoader::GameLoader(library::UserLibraryService &library,
                         library::EntryResolver &resolver,
                         settings::SettingsService &settingsService,
                         const EmulationContext &context)
    : m_library(library), m_resolver(resolver),
      m_settingsService(settingsService), m_context(context) {
  }

  GameLoadResult GameLoader::load(const int entryId, LaunchOverrides launch,
                                  const CoreFactory &factory) {
    spdlog::info("[GameLoader] Loading entry with id {}", entryId);

    auto entry = m_library.getEntry(entryId);
    if (!entry.has_value()) {
      spdlog::warn("[GameLoader] Entry with id {} does not exist", entryId);
      return {};
    }

    const auto resolved = m_resolver.resolve(*entry);
    if (!resolved.valid) {
      spdlog::warn("[GameLoader] No usable content for entry with id {}", entryId);
      return {};
    }

    const auto &contentFile = resolved.contentFile;

    const auto contentPath =
        contentFile.m_inArchive ? contentFile.m_archivePathName : contentFile.m_filePath;
    if (!std::filesystem::exists(contentPath)) {
      spdlog::error("[GameLoader] Content path doesn't exist: {}", contentPath);
      return {};
    }

    library::ContentLoader contentLoader;
    auto loaded = contentLoader.load(contentFile);
    if (!loaded.valid) {
      spdlog::error("[GameLoader] Failed to load content for entry {}", entryId);
      return {};
    }

    if (resolved.patch.has_value()) {
      auto patch = *resolved.patch;
      if (!patch.load()) {
        // The entry resolved to a patched version; running it unpatched would be
        // wrong (and could corrupt saves), so treat this as a load failure
        spdlog::error("[GameLoader] Failed to load patch {} for entry {}",
                      patch.m_filePath, entryId);
        return {};
      }
      contentLoader.applyPatch(loaded, contentFile.m_platformId, patch);
    }

    const int saveSlot = launch.saveSlot >= 0
                           ? launch.saveSlot
                           : static_cast<int>(entry->activeSaveSlot);

    QByteArray saveDataBytes;
    if (const auto saveManager = m_context.saveManager) {
      const auto saveData = saveManager->readSaveData(loaded.contentHash, saveSlot);
      if (saveData.has_value()) {
        saveDataBytes = QByteArray(saveData->getSaveRamData().data(),
                                   saveData->getSaveRamData().size());
      }
    }

    GameLoadResult result;
    result.entry = entry.value();
    result.contentHash = result.entry.contentHash;

    if (m_context.platformService) {
      if (auto platform = m_context.platformService->getPlatform(
        result.entry.platformId)) {
        result.platform = platform.value();
      }
    }

    // Resolve the core for this entry (default -> per-platform -> per-game
    // override) and locate its DLL
    const auto coreName = CoreRegistry::instance().resolveCoreName(
      result.entry.platformId, result.contentHash, &m_settingsService);
    const std::string corePath = CoreRegistry::instance().dllPathFor(coreName);
    const auto &catalog = settings::SettingsCatalog::instance();
    auto coreConfig = std::make_shared<CoreConfiguration>(
      result.entry.contentHash, result.entry.platformId,
      catalog.settingsForCore(coreName), catalog.coreDefaults(coreName),
      m_settingsService);
    result.coreConfig = coreConfig;

    std::string saveDirectory;
    if (const auto saveManager = m_context.saveManager) {
      saveDirectory = saveManager->getSaveDirectory();
    }

    std::unique_ptr<::libretro::ICore> core;
    try {
      core = factory(firelight::libretro::CoreRunConfig{
        .platformId = static_cast<int>(result.entry.platformId),
        .corePath = corePath,
        .configProvider = coreConfig,
        .systemDirectory = m_context.coreSystemDirectory,
        .saveDirectory = saveDirectory
      });
    } catch (const std::exception &e) {
      spdlog::error("[GameLoader] Failed to load core for entry {}: {}",
                    entryId, e.what());
      return {};
    }
    if (!core) {
      spdlog::error("[GameLoader] Core factory returned null for entry {}",
                    entryId);
      return {};
    }

    result.instance = std::make_unique<EmulatorInstance>(
      std::move(core), contentFile.m_filePath, result.contentHash,
      result.entry.platformId, saveSlot, std::move(loaded.contentBytes),
      std::vector<uint8_t>(saveDataBytes.begin(), saveDataBytes.end()), m_context);

    // The instance is born muted (if requested) once initialize() creates its
    // AudioManager
    result.instance->setStartMuted(launch.muted);

    result.success = true;
    return result;
  }
} // namespace firelight::emulation

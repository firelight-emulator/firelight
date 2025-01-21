//
// Created by alexs on 10/15/2024.
//

#include "game_loader.hpp"

#include <qtconcurrentrun.h>

#include "platform_metadata.hpp"

namespace firelight {
    GameLoader::GameLoader() {
        m_threadPool.setMaxThreadCount(1);
    }

    void GameLoader::loadEntry(int entryId, bool waitForApproval) {
      printf("Calling load entry\n");
        // TODO: Figure out how to prevent stutter here.
        m_threadPool.start([this, entryId, waitForApproval] {
                if (waitForApproval) {
                    m_holding = true;
                }

                spdlog::info("Loading entry with id {}", entryId);

                auto entry = getUserLibrary()->getEntry(entryId);

                // auto entry = getLibraryDatabase()->getLibraryEntry(entryId);
                //
                if (!entry.has_value()) {
                    spdlog::warn("Entry with id {} does not exist...", entryId);
                }
                //
                // m_currentEntry = *entry;

                auto roms = getUserLibrary()->getRomFilesWithContentHash(entry->contentHash);
                if (roms.empty()) {
                    spdlog::warn("No ROM file found for entry with id {}", entryId);
                }

                auto romFile = std::unique_ptr<firelight::library::RomFile>();
                for (auto &rom: roms) {
                    if (!rom.inArchive()) {
                        romFile = std::make_unique<firelight::library::RomFile>(rom);
                    }
                }

                // If we didn't find one that isn't in an archive, just use the first one
                if (!romFile) {
                    romFile = std::make_unique<firelight::library::RomFile>(roms[0]);
                }

                if (romFile->inArchive() && !std::filesystem::exists(romFile->getArchivePathName().toStdString())) {
                    printf("content path doesn't exist\n");
                    return;
                }

                if (!romFile->inArchive() && !std::filesystem::exists(romFile->getFilePath().toStdString())) {
                    printf("content path doesn't exist\n");
                    return;
                }

                romFile->load();

                std::string corePath = firelight::PlatformMetadata::getCoreDllPath(entry->platformId);
                //
                QByteArray saveDataBytes;
                const auto saveData = getSaveManager()->readSaveData(romFile->getContentHash(), entry->activeSaveSlot);
                if (saveData.has_value()) {
                    saveDataBytes = QByteArray(saveData->getSaveRamData().data(),
                                               saveData->getSaveRamData().size());
                }

                if (!m_holding) {
                    emit gameLoaded(
                        entryId,
                        romFile->getContentBytes(),
                        saveDataBytes,
                        QString::fromStdString(corePath),
                        romFile->getContentHash(),
                        entry->activeSaveSlot,
                        entry->platformId,
                        romFile->getFilePath()
                    );
                } else {
                    m_entryId = entryId;
                    m_gameData = romFile->getContentBytes();
                    m_saveData = saveDataBytes;
                    m_corePath = QString::fromStdString(corePath);
                    m_contentHash = romFile->getContentHash();
                    m_saveSlotNumber = entry->activeSaveSlot;
                    m_platformId = entry->platformId;
                    m_contentPath = romFile->getFilePath();

                    m_lastLoadSuccessful = true;
                }
            }
        );
    }

    void GameLoader::approve() {
        m_holding = false;
        if (m_lastLoadSuccessful) {
            emit gameLoaded(
                m_entryId,
                m_gameData,
                m_saveData,
                m_corePath,
                m_contentHash,
                m_saveSlotNumber,
                m_platformId,
                m_contentPath
            );

            m_lastLoadSuccessful = false;
        }
    }

    void GameLoader::emitResult() {
    }
} // firelight

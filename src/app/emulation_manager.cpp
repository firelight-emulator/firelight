#include "emulation_manager.hpp"

#include "rcheevos/ra_client.hpp"
#include "audio_manager.hpp"
#include "emulator_renderer.hpp"
#include "input/controller_manager.hpp"
#include "patching/bps_patch.hpp"
#include "patching/ips_patch.hpp"
#include "patching/pm_star_rod_mod_patch.hpp"
#include "patching/rom_patch.hpp"
#include "patching/ups_patch.hpp"
#include "patching/yay_0_codec.hpp"
#include "saves/save_manager.hpp"
#include "saves/savefile.hpp"

#include <QPainter>
#include <QtConcurrent>
#include <firelight/userdata_database.hpp>
#include <fstream>
#include <firelight/library/rom_file.hpp>
#include <spdlog/spdlog.h>

#include "platform_metadata.hpp"

constexpr int SAVE_FREQUENCY_MILLIS = 10000;

EmulationManager::EmulationManager(QQuickItem *parent)
  : QQuickFramebufferObject(parent) {
  setTextureFollowsItemSize(false);
  setMirrorVertically(true);
  setFlag(ItemHasContents);

  // connect(this, &EmulationManager::emulationStarted, this,
  //         &QQuickFramebufferObject::update, Qt::QueuedConnection);
  // connect(this, &EmulationManager::gamePaused, this,
  //         &QQuickFramebufferObject::update, Qt::QueuedConnection);
  // connect(this, &EmulationManager::gameResumed, this,
  //         &QQuickFramebufferObject::update, Qt::QueuedConnection);

  // connect(
  //   this, &EmulationManager::gameLoadSucceeded, this,
  //   [this] {
  //     m_currentPlaySession = std::make_unique<firelight::db::PlaySession>();
  //     m_currentPlaySession->contentId = m_currentEntry.contentId;
  //     m_currentPlaySession->startTime = QDateTime::currentMSecsSinceEpoch();
  //     m_currentPlaySession->slotNumber = m_currentEntry.activeSaveSlot;
  //
  //     m_playtimeTimer.start();
  //     QMetaObject::invokeMethod(&m_autosaveTimer, "start", Qt::QueuedConnection);
  //
  //     emit emulationStarted();
  //   },
  //   Qt::QueuedConnection);

  // m_autosaveTimer.setInterval(SAVE_FREQUENCY_MILLIS);
  // m_autosaveTimer.setSingleShot(false);
  // m_autosaveTimer.callOnTimeout([this] {
  //   spdlog::info("Autosaving SRAM data (interval {}ms)", SAVE_FREQUENCY_MILLIS);
  //   save();
  // });
}

EmulationManager::~EmulationManager() {
  printf("Destroying EmulationManager\n");

  // QMetaObject::invokeMethod(&m_autosaveTimer, "stop", Qt::QueuedConnection);
  //
  // if (m_currentPlaySession) {
  //   m_currentPlaySession->endTime = QDateTime::currentMSecsSinceEpoch();
  //
  //   const auto timerValue = m_playtimeTimer.restart();
  //   if (!m_paused) {
  //     m_currentPlaySession->unpausedDurationMillis += timerValue;
  //   }
  //
  //   const auto session = m_currentPlaySession.release();
  //   getUserdataManager()->createPlaySession(*session);
  // }
  //
  // save(true);

  // getAchievementManager()->unloadGame();
}

QQuickFramebufferObject::Renderer *EmulationManager::createRenderer() const {
  return new EmulatorRenderer([this] {
    const_cast<EmulationManager *>(this)->update();
  });
}


QString EmulationManager::currentGameName() const {
  return QString::fromStdString("");
}

int EmulationManager::nativeWidth() const { return m_nativeWidth; }
int EmulationManager::nativeHeight() const { return m_nativeHeight; }

float EmulationManager::nativeAspectRatio() const {
  return m_nativeAspectRatio;
}

float EmulationManager::playSpeed() const {
  return m_playSpeed;
}

void EmulationManager::setPlaySpeed(float speed) {
  m_playSpeed = speed;
  update();
}

void EmulationManager::pauseGame() {
  m_paused = true;
  update();
}

void EmulationManager::resumeGame() {
  m_paused = false;
  update();
}

void EmulationManager::resetEmulation() {
  m_shouldReset = true;
  update();
}

void EmulationManager::createSuspendPoint() {
  m_shouldCreateSuspendPoint = true;
  update();
}

void EmulationManager::createRewindPoints() {
  m_shouldGetRewindPoints = true;
  update();
}

void EmulationManager::loadRewindPoint(int index) {
  m_rewindPointIndex = index;
  update();
}

void EmulationManager::writeSuspendPoint(int index) {
  m_writeSuspendPointIndex = index;
  update();
}

void EmulationManager::loadSuspendPoint(int index) {
  m_loadSuspendPointIndex = index;
  update();
}

void EmulationManager::loadLastSuspendPoint() {
  m_shouldLoadLastSuspendPoint = true;
  update();
}

void EmulationManager::setGeometry(int nativeWidth, int nativeHeight, float nativeAspectRatio) {
  if (m_nativeWidth != nativeWidth) {
    m_nativeWidth = nativeWidth;
    emit nativeWidthChanged();
  }

  if (m_nativeHeight != nativeHeight) {
    m_nativeHeight = nativeHeight;
    emit nativeHeightChanged();
  }

  if (m_nativeAspectRatio != nativeAspectRatio) {
    m_nativeAspectRatio = nativeAspectRatio;
    emit nativeAspectRatioChanged();
  }
}

void EmulationManager::setIsRunning(bool running) {
  if (m_running != running) {
    m_running = running;
    if (m_running) {
      emit emulationStarted();
    } else {
      emit emulationStopped();
    }
  }
}

void EmulationManager::loadLibraryEntry(int entryId) {
  QThreadPool::globalInstance()->start([this, entryId] {
      spdlog::info("Loading entry with id {}", entryId);

      auto entry = getUserLibrary()->getEntry(entryId);

      // auto entry = getLibraryDatabase()->getLibraryEntry(entryId);
      //
      if (!entry.has_value()) {
        emit gameLoadFailed();
        spdlog::warn("Entry with id {} does not exist...", entryId);
      }
      //
      // m_currentEntry = *entry;
      emit currentGameNameChanged();

      auto roms = getUserLibrary()->getRomFilesWithContentHash(entry->contentHash);
      if (roms.empty()) {
        emit gameLoadFailed();
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
        emit gameLoadFailed();
        return;
      }

      if (!romFile->inArchive() && !std::filesystem::exists(romFile->getFilePath().toStdString())) {
        printf("content path doesn't exist\n");
        emit gameLoadFailed();
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
      //
      // auto rom = firelight::library::RomFile(QString::fromStdString(entry->contentPath));
      //
      m_contentPath = romFile->getFilePath();
      m_gameData = QByteArray(romFile->getContentBytes());
      m_saveData = saveDataBytes;
      m_corePath = QString::fromStdString(corePath);
      m_saveSlotNumber = entry->activeSaveSlot;
      m_platformId = entry->platformId;
      m_contentHash = romFile->getContentHash();

      m_gameReady = true;
      update();

      // if (entry->type == firelight::db::LibraryEntry::EntryType::PATCH) {
      //   if (entry->parentEntryId == -1) {
      //     printf("no parent entry id\n");
      //     emit gameLoadFailed();
      //     return;
      //   }
      //
      //   auto parent = getLibraryDatabase()->getLibraryEntry(entry->parentEntryId);
      //   if (!parent.has_value()) {
      //     // TODO: Update patch to not have a parent
      //     printf("parent entry doesn't exist\n");
      //     emit gameLoadFailed();
      //     return;
      //   }
      //
      //   auto size = std::filesystem::file_size(parent->contentPath);
      //
      //   std::vector<char> gameDataVec(size);
      //   std::ifstream file(parent->contentPath, std::ios::binary);
      //
      //   file.read(gameDataVec.data(), size);
      //   file.close();
      //
      //   std::vector<uint8_t> gameDataVecUint8(gameDataVec.begin(),
      //                                         gameDataVec.end());
      //
      //   firelight::patching::IRomPatch *patch = nullptr;
      //   auto ext = std::filesystem::path(entry->contentPath).extension();
      //   if (ext == ".bps") {
      //     patch = new firelight::patching::BPSPatch(entry->contentPath);
      //   } else if (ext == ".ips") {
      //     patch = new firelight::patching::IPSPatch(entry->contentPath);
      //   } else if (ext == ".mod") {
      //     auto decompressed =
      //         firelight::patching::Yay0Codec::decompress(gameDataVecUint8.data());
      //
      //     patch = new firelight::patching::PMStarRodModPatch(decompressed);
      //   } else if (ext == ".ups") {
      //     patch = new firelight::patching::UPSPatch(entry->contentPath);
      //   }
      //
      //   if (patch == nullptr) {
      //     // TODO: Actual error
      //     printf("patch is null\n");
      //     emit gameLoadFailed();
      //     return;
      //   }
      //
      //   auto patchedGame = patch->patchRom(gameDataVecUint8);
      //
      //   auto gameData = QByteArray(reinterpret_cast<char *>(patchedGame.data()),
      //                              patchedGame.size());
      //
      //   QByteArray saveDataBytes;
      //   const auto saveData = getSaveManager()->readSaveDataForEntry(*entry);
      //   if (saveData.has_value()) {
      //     saveDataBytes = QByteArray(saveData->getSaveRamData().data(),
      //                                saveData->getSaveRamData().size());
      //   }
      //
      //   std::string corePath = firelight::PlatformMetadata::getCoreDllPath(parent->platformId);
      //   m_gameData = QByteArray(gameData.data(), gameData.size());
      //   m_saveData = saveDataBytes;
      //   m_corePath = QString::fromStdString(corePath);
      // } else {

      // }
      // m_core = std::make_unique<libretro::Core>(m_corePath.toStdString());
      //
      // m_core->setAudioReceiver(std::make_shared<AudioManager>());
      // m_core->setRetropadProvider(getControllerManager());
      //
      // m_core->setSystemDirectory("./system");
      // // m_core->setSaveDirectory(".");
      // m_core->init();
      //
      // libretro::Game game(
      //   entry->contentPath,
      //   vector<unsigned char>(m_gameData.begin(), m_gameData.end()));
      // m_core->loadGame(&game);
      //
      // if (m_saveData.size() > 0) {
      //   m_core->writeMemoryData(libretro::SAVE_RAM,
      //                           vector(m_saveData.begin(), m_saveData.end()));
      // }

      // auto md5 = calculateMD5(m_gameData.data(), m_gameData.size());

      // emit gameLoadSucceeded();
    }

  );
}

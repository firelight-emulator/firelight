#include "emulation_manager.hpp"

#include "achieve/ra_client.hpp"
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
#include <QQuickWindow>

#include <QPainter>
#include <QtConcurrent>
#include <firelight/userdata_database.hpp>
#include <fstream>
#include <spdlog/spdlog.h>

#include "platform_metadata.hpp"

constexpr int SAVE_FREQUENCY_MILLIS = 10000;

EmulationManager::EmulationManager(QQuickItem *parent)
  : QQuickFramebufferObject(parent) {
  printf("Creating EmulationManager\n");
  setTextureFollowsItemSize(false);
  setMirrorVertically(true);
  setFlag(ItemHasContents);
}

EmulationManager::~EmulationManager() {
  printf("Destroying EmulationManager\n");
}

QQuickFramebufferObject::Renderer *EmulationManager::createRenderer() const {
  return new EmulatorRenderer();
}


QString EmulationManager::currentGameName() const {
  return QString::fromStdString(m_currentEntry.displayName);
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
    auto entry = getLibraryDatabase()->getLibraryEntry(entryId);

    if (!entry.has_value()) {
      emit gameLoadFailed();
      spdlog::warn("Entry with id {} does not exist...", entryId);
    }

    m_currentEntry = *entry;
    emit currentGameNameChanged();

    if (!std::filesystem::exists(entry->contentPath)) {
      printf("content path doesn't exist\n");
      emit gameLoadFailed();
      return;
    }

    if (entry->type == firelight::db::LibraryEntry::EntryType::PATCH) {
      if (entry->parentEntryId == -1) {
        printf("no parent entry id\n");
        emit gameLoadFailed();
        return;
      }

      auto parent = getLibraryDatabase()->getLibraryEntry(entry->parentEntryId);
      if (!parent.has_value()) {
        // TODO: Update patch to not have a parent
        printf("parent entry doesn't exist\n");
        emit gameLoadFailed();
        return;
      }

      auto size = std::filesystem::file_size(parent->contentPath);

      std::vector<char> gameDataVec(size);
      std::ifstream file(parent->contentPath, std::ios::binary);

      file.read(gameDataVec.data(), size);
      file.close();

      std::vector<uint8_t> gameDataVecUint8(gameDataVec.begin(),
                                            gameDataVec.end());

      firelight::patching::IRomPatch *patch = nullptr;
      auto ext = std::filesystem::path(entry->contentPath).extension();
      if (ext == ".bps") {
        patch = new firelight::patching::BPSPatch(entry->contentPath);
      } else if (ext == ".ips") {
        patch = new firelight::patching::IPSPatch(entry->contentPath);
      } else if (ext == ".mod") {
        auto decompressed =
            firelight::patching::Yay0Codec::decompress(gameDataVecUint8.data());

        patch = new firelight::patching::PMStarRodModPatch(decompressed);
      } else if (ext == ".ups") {
        patch = new firelight::patching::UPSPatch(entry->contentPath);
      }

      if (patch == nullptr) {
        // TODO: Actual error
        printf("patch is null\n");
        emit gameLoadFailed();
        return;
      }

      auto patchedGame = patch->patchRom(gameDataVecUint8);

      auto gameData = QByteArray(reinterpret_cast<char *>(patchedGame.data()),
                                 patchedGame.size());

      QByteArray saveDataBytes;
      const auto saveData = getSaveManager()->readSaveDataForEntry(*entry);
      if (saveData.has_value()) {
        saveDataBytes = QByteArray(saveData->getSaveRamData().data(),
                                   saveData->getSaveRamData().size());
      }

      std::string corePath = firelight::PlatformMetadata::getCoreDllPath(parent->platformId);
      m_gameData = QByteArray(gameData.data(), gameData.size());
      m_saveData = saveDataBytes;
      m_corePath = QString::fromStdString(corePath);
    } else {
      std::string corePath = firelight::PlatformMetadata::getCoreDllPath(entry->platformId);
      auto size = std::filesystem::file_size(entry->contentPath);

      QByteArray saveDataBytes;
      const auto saveData = getSaveManager()->readSaveDataForEntry(*entry);
      if (saveData.has_value()) {
        saveDataBytes = QByteArray(saveData->getSaveRamData().data(),
                                   saveData->getSaveRamData().size());
      }

      std::vector<char> gameDataVec(size);
      std::ifstream file(entry->contentPath, std::ios::binary);

      file.read(gameDataVec.data(), size);
      file.close();

      m_gameData = QByteArray(gameDataVec.data(), gameDataVec.size());
      m_saveData = saveDataBytes;
      m_corePath = QString::fromStdString(corePath);

      m_gameReady = true;
    }

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
  });
}

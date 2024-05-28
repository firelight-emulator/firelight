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

#include <QPainter>
#include <QtConcurrent>
#include <firelight/userdata_database.hpp>
#include <fstream>
#include <spdlog/spdlog.h>
#include <utility>

constexpr int SAVE_FREQUENCY_MILLIS = 10000;

EmulationManager::EmulationManager(QQuickItem *parent)
  : QQuickFramebufferObject(parent) {
  setTextureFollowsItemSize(false);
  setMirrorVertically(true);
  setFlag(ItemHasContents);

  connect(this, &EmulationManager::emulationStarted, this,
          &QQuickFramebufferObject::update, Qt::QueuedConnection);
  connect(this, &EmulationManager::gamePaused, this,
          &QQuickFramebufferObject::update, Qt::QueuedConnection);
  connect(this, &EmulationManager::gameResumed, this,
          &QQuickFramebufferObject::update, Qt::QueuedConnection);

  connect(
    this, &EmulationManager::gameLoadSucceeded, this,
    [this] {
      m_gameLoadedSignalReady = true;
      if (m_achievementsLoadedSignalReady) {
        emit readyToStart();
        m_gameLoadedSignalReady = false;
        m_achievementsLoadedSignalReady = false;
      }
    },
    Qt::QueuedConnection);

  connect(
    getAchievementManager(),
    &firelight::achievements::RAClient::gameLoadSucceeded, this,
    [this] {
      m_achievementsLoadedSignalReady = true;
      if (m_gameLoadedSignalReady) {
        emit readyToStart();
        m_gameLoadedSignalReady = false;
        m_achievementsLoadedSignalReady = false;
      }
    },
    Qt::QueuedConnection);
}

EmulationManager::~EmulationManager() {
  if (!m_isRunning) {
    return;
  }

  m_isRunning = false;

  if (m_currentPlaySession) {
    m_currentPlaySession->endTime = QDateTime::currentMSecsSinceEpoch();

    const auto timerValue = m_playtimeTimer.restart();
    if (!m_paused) {
      m_currentPlaySession->unpausedDurationMillis += timerValue;
    }

    const auto session = m_currentPlaySession.release();
    getUserdataManager()->createPlaySession(*session);
  }

  save(true);

  if (m_core) {
    // m_core->unloadGame();
    // m_core->deinit();
    // m_core.reset();
  }

  printf("End of emulation manager destructor\n");
}

QQuickFramebufferObject::Renderer *EmulationManager::createRenderer() const {
  return new EmulatorRenderer(this);
}

void EmulationManager::setGetProcAddressFunction(
  const std::function<proc_address_t(const char *)> &getProcAddressFunction) {
  m_getProcAddressFunction = getProcAddressFunction;
}

std::function<void()> EmulationManager::consumeContextResetFunction() {
  if (m_resetContextFunction) {
    printf("giving it\n");
    auto func = m_resetContextFunction;
    m_resetContextFunction = nullptr;
    return func;
  }

  return nullptr;
}

std::function<void()> EmulationManager::consumeContextDestroyFunction() {
  if (m_destroyContextFunction) {
    auto func = m_destroyContextFunction;
    m_destroyContextFunction = nullptr;
    return func;
  }

  return nullptr;
}

void EmulationManager::setReceiveVideoDataFunction(
  const std::function<void(const void *data, unsigned width, unsigned height,
                           size_t pitch)> &receiveVideoDataFunction) {
  m_receiveVideoDataFunction = receiveVideoDataFunction;
}

void EmulationManager::setCurrentFboId(const int fboId) {
  m_currentFboId = fboId;
}

QString EmulationManager::currentGameName() const {
  return QString::fromStdString(m_currentEntry.displayName);
}

int EmulationManager::nativeWidth() const { return m_nativeWidth; }
int EmulationManager::nativeHeight() const { return m_nativeHeight; }

float EmulationManager::nativeAspectRatio() const {
  return m_nativeAspectRatio;
}

void EmulationManager::receive(const void *data, unsigned width,
                               unsigned height, size_t pitch) {
  if (!m_usingHwRendering && m_receiveVideoDataFunction && data != nullptr) {
    m_receiveVideoDataFunction(data, width, height, pitch);
  }
}

proc_address_t EmulationManager::getProcAddress(const char *sym) {
  if (!m_getProcAddressFunction) {
    return nullptr;
  }

  return m_getProcAddressFunction(sym);
}

void EmulationManager::setResetContextFunc(context_reset_func resetFunction) {
  printf("Setting reset context function\n");
  m_usingHwRendering = true;
  m_resetContextFunction = resetFunction;
}

void EmulationManager::setDestroyContextFunc(
  context_destroy_func destroyFunction) {
  printf("Setting destroy context function\n");
  m_usingHwRendering = true;
  m_destroyContextFunction = destroyFunction;
}

void EmulationManager::pauseGame() {
  if (!m_isRunning) {
    return;
  }

  if (!m_paused) {
    m_currentPlaySession->unpausedDurationMillis += m_playtimeTimer.restart();
    emit gamePaused();
  }

  m_paused = true;
}

void EmulationManager::resumeGame() {
  if (!m_isRunning) {
    return;
  }

  if (m_paused) {
    m_playtimeTimer.restart();
    emit gameResumed();
  }

  m_paused = false;
}

void EmulationManager::startEmulation() {
  if (m_isRunning) {
    return;
  }

  QThreadPool::globalInstance()->start([this] {
    m_currentPlaySession = std::make_unique<firelight::db::PlaySession>();
    m_currentPlaySession->contentId = m_currentEntry.contentId;
    m_currentPlaySession->startTime = QDateTime::currentMSecsSinceEpoch();
    m_currentPlaySession->slotNumber = m_currentEntry.activeSaveSlot;

    m_isRunning = true;
    m_paused = false;
    m_playtimeTimer.start();

    emit emulationStarted();
  });
}

void EmulationManager::stopEmulation() {
  if (!m_isRunning) {
    return;
  }

  QThreadPool::globalInstance()->start([this] {
    m_currentPlaySession->endTime = QDateTime::currentMSecsSinceEpoch();

    const auto timerValue = m_playtimeTimer.restart();
    if (!m_paused) {
      m_currentPlaySession->unpausedDurationMillis += timerValue;
    }

    const auto session = m_currentPlaySession.get();
    getUserdataManager()->createPlaySession(*session);
    m_currentPlaySession.reset();

    getAchievementManager()->unloadGame();
    m_achievementsLoadedSignalReady = false;
    m_gameLoadedSignalReady = false;

    m_isRunning = false;
    save(true);
    m_nativeWidth = 0;
    m_nativeHeight = 0;
    m_nativeAspectRatio = 0;

    emit nativeWidthChanged();
    emit nativeHeightChanged();

    // if (m_destroyContextFunction) {
    //   m_destroyContextFunction();
    // }
    shouldUnload = true;

    m_usingHwRendering = false;
    // m_core->unloadGame();
    // m_core->deinit();
    // m_core.reset();

    emit emulationStopped();
  });

  update();
}

void EmulationManager::resetEmulation() {
  if (m_core) {
    m_core->reset();
    update();
  }
}

bool EmulationManager::isRunning() const { return m_isRunning; }

void EmulationManager::save(const bool waitForFinish) {
  spdlog::debug("Autosaving SRAM data (interval {}ms)", SAVE_FREQUENCY_MILLIS);
  firelight::saves::Savefile saveData(
    m_core->getMemoryData(libretro::SAVE_RAM));
  // saveData.setImage(m_fbo->toImage());

  QFuture<bool> result =
      getSaveManager()->writeSaveDataForEntry(m_currentEntry, saveData);

  if (waitForFinish) {
    result.waitForFinished();
  }
}

uintptr_t EmulationManager::getCurrentFramebufferId() { return m_currentFboId; }

void EmulationManager::setSystemAVInfo(retro_system_av_info *info) {
  const auto width = info->geometry.base_width;
  const auto height = info->geometry.base_height;

  if (width > 0 && height > 0) {
    if (width != m_nativeWidth) {
      m_nativeWidth = width;
      emit nativeWidthChanged();
    }

    if (height != m_nativeHeight) {
      m_nativeHeight = height;
      emit nativeHeightChanged();
    }

    const auto aspectRatio =
        static_cast<float>(width) / static_cast<float>(height);
    if (aspectRatio != m_nativeAspectRatio) {
      m_nativeAspectRatio = aspectRatio;
      emit nativeAspectRatioChanged();
    }
  }
}

bool EmulationManager::runFrame() {
  if (shouldUnload) {
    m_core->unloadGame();
    m_core->deinit();
    m_core.reset();

    shouldUnload = false;
  }

  if (m_isRunning && !m_paused) {
    m_core->run(0);
    getAchievementManager()->doFrame(m_core.get(), m_currentEntry);
    return true;
  }

  return false;

  //
  // if (m_running) {
  //   if (!m_paused) {
  //     const auto frameBegin = SDL_GetPerformanceCounter();
  //     lastTick = thisTick;
  //     thisTick = SDL_GetPerformanceCounter();
  //
  //     auto deltaTime =
  //         (thisTick - lastTick) * 1000 /
  //         (double)SDL_GetPerformanceFrequency();
  //
  //     m_millisSinceLastSave += static_cast<int>(deltaTime);
  //     if (m_millisSinceLastSave < 0) {
  //       m_millisSinceLastSave = 0;
  //     }
  //
  //     if (m_millisSinceLastSave >= SAVE_FREQUENCY_MILLIS) {
  //       m_millisSinceLastSave = 0;
  //       // gameImage = gameFbo->toImage();
  //
  //       const auto state = core->serializeState();
  //       const SuspendPoint suspendPoint{
  //           .state = state, .timestamp =
  //           QDateTime::currentMSecsSinceEpoch()};
  //
  //       m_suspendPoints.push_back(suspendPoint);
  //     }
  //
  //     frameCount++;
  //     if (frameSkipRatio == 0 || (frameCount % frameSkipRatio == 0)) {
  //       core->run(deltaTime);
  //
  //       auto frameEnd = SDL_GetPerformanceCounter();
  //       auto frameDiff = ((frameEnd - frameBegin) * 1000 /
  //                         static_cast<double>(SDL_GetPerformanceFrequency()));
  //       totalFrameWorkDurationMillis += frameDiff;
  //       numFrames++;
  //
  //       if (numFrames == 300) {
  //         printf("Average frame work duration: %fms\n",
  //                totalFrameWorkDurationMillis / numFrames);
  //         totalFrameWorkDurationMillis = 0;
  //         numFrames = 0;
  //       }
  //     }
  //     update();
  //   }
  //   m_ranLastFrame = true;
  //
  //   // printf("Serialize size: %lu\n", core->getSerializeSize());
  //
  //   if (m_fbo != nullptr) {
  //     const auto image = m_fbo->toImage();
  //     // printf("image size bytes: %lld\n", image.sizeInBytes());
  //
  //     // Get a pointer to the raw data
  //     // auto future = QtConcurrent::run([image] {
  //     //   const uchar *data = image.constBits();
  //     //   // Compress the image data using zlib
  //     //   uLongf compressedDataSize = compressBound(image.sizeInBytes());
  //     //   auto *compressedData = new uchar[compressedDataSize];
  //     //   if (compress2(compressedData, &compressedDataSize, data,
  //     //                 image.sizeInBytes(), Z_BEST_COMPRESSION) != Z_OK) {
  //     //     // printf("Failed to compress image data\n");
  //     //   } else {
  //     //     printf("Compressed image size bytes: %lu\n",
  //     compressedDataSize);
  //     //     // Now you can use 'compressedData' to transmit the compressed
  //     //     // image
  //     //     //     // over a network connection Be sure to also transmit the
  //     //     size
  //     //     //     of the
  //     //     // compressed data, which is compressedDataSize
  //     //   }
  //     //
  //     //   delete[] compressedData;
  //     // });
  //
  //     // Now you can use 'data' to transmit the image over a network
  //     // connection
  //     // Be sure to also transmit the size of the data, which is
  //     // image.sizeInBytes()
  //   }
  // }
}

void EmulationManager::loadLibraryEntry(int entryId) {
  m_gameLoadedSignalReady = false;
  m_achievementsLoadedSignalReady = false;

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

      std::string corePath;
      if (parent->platformId == 7) {
        corePath = "./system/_cores/mupen64plus_next_libretro.dll";
      } else if (parent->platformId == 6) {
        corePath = "./system/_cores/snes9x_libretro.dll";
      } else if (parent->platformId == 2) {
        corePath = "./system/_cores/gambatte_libretro.dll";
      } else if (parent->platformId == 1) {
        corePath = "./system/_cores/gambatte_libretro.dll";
      } else if (parent->platformId == 3) {
        corePath = "./system/_cores/mgba_libretro.dll";
      } else if (parent->platformId == 10) {
        corePath = "./system/_cores/melondsds_libretro.dll";
      } else if (parent->platformId == 13) {
        corePath = "./system/_cores/genesis_plus_gx_libretro.dll";
      }

      m_gameData = QByteArray(gameData.data(), gameData.size());
      m_saveData = saveDataBytes;
      m_corePath = QString::fromStdString(corePath);
    } else {
      std::string corePath;
      if (entry->platformId == 7) {
        corePath = "./system/_cores/mupen64plus_next_libretro.dll";
      } else if (entry->platformId == 6) {
        corePath = "./system/_cores/snes9x_libretro.dll";
      } else if (entry->platformId == 2) {
        corePath = "./system/_cores/gambatte_libretro.dll";
      } else if (entry->platformId == 1) {
        corePath = "./system/_cores/gambatte_libretro.dll";
      } else if (entry->platformId == 3) {
        corePath = "./system/_cores/mgba_libretro.dll";
      } else if (entry->platformId == 10) {
        corePath = "./system/_cores/melondsds_libretro.dll";
      } else if (entry->platformId == 13) {
        corePath = "./system/_cores/genesis_plus_gx_libretro.dll";
      }

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
    }


    printf("Initializing on thread: %p\n", QThread::currentThreadId());

    m_core = std::make_unique<libretro::Core>(m_corePath.toStdString());

    m_core->setVideoReceiver(this);
    m_core->setAudioReceiver(new AudioManager());
    m_core->setRetropadProvider(getControllerManager());

    m_core->setSystemDirectory("./system");
    // m_core->setSaveDirectory(".");

    // m_core->setVideoReceiver(this);
    // m_core->setAudioReceiver(new AudioManager());
    m_core->init();

    libretro::Game game(
      entry->contentPath,
      vector<unsigned char>(m_gameData.begin(), m_gameData.end()));
    m_core->loadGame(&game);

    if (m_saveData.size() > 0) {
      m_core->writeMemoryData(libretro::SAVE_RAM,
                              vector(m_saveData.begin(), m_saveData.end()));
    }

    emit gameLoadSucceeded();
  });
}

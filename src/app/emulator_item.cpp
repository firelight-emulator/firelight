#include "emulator_item.hpp"
#include <QQuickWindow>
#include <rhi/qrhi_platform.h>
#include <spdlog/spdlog.h>

#include "emulator_item_renderer.hpp"
#include "platform_metadata.hpp"

void EmulatorItem::mouseMoveEvent(QMouseEvent *event) {
  const auto pos = event->position();
  const auto bounds = boundingRect();

  const auto x = (pos.x() - bounds.width() / 2) / (bounds.width() / 2);
  const auto y = (pos.y() - bounds.height() / 2) / (bounds.height() / 2);

  getInputManager()->updateMouseState(x, y, m_mousePressed);
}

EmulatorItem::EmulatorItem(QQuickItem *parent) : QQuickRhiItem(parent) {
  setFlag(ItemHasContents);
  setAcceptHoverEvents(true);
  setAcceptTouchEvents(true);
  setAcceptedMouseButtons(Qt::LeftButton);

  m_threadPool.setMaxThreadCount(1);

  m_autosaveTimer.setInterval(10000);
  m_autosaveTimer.setSingleShot(false);
  connect(&m_autosaveTimer, &QTimer::timeout, [this] {
    if (m_renderer) {
      m_renderer->submitCommand({.type = EmulatorItemRenderer::WriteSaveFile});
      update();
    }
  });

  m_autosaveTimer.start();

  m_rewindPointTimer.setInterval(3000);
  m_rewindPointTimer.setSingleShot(false);
  connect(&m_rewindPointTimer, &QTimer::timeout, [this] {
    if (m_renderer) {
      m_renderer->submitCommand(
          {.type = EmulatorItemRenderer::WriteRewindPoint});
      update();
    }
  });
  m_rewindPointTimer.start();
}

EmulatorItem::~EmulatorItem() {
  spdlog::info("Destroying EmulatorItem");
  m_autosaveTimer.stop();
}

bool EmulatorItem::paused() const { return m_paused; }

void EmulatorItem::setPaused(const bool paused) {
  if (m_paused != paused) {
    m_paused = paused;
    m_audioManager->setMuted(m_paused);
    emit pausedChanged();
    update();
  }
}

float EmulatorItem::audioBufferLevel() const {
  return m_audioManager ? m_audioManager->getBufferLevel() : 0.0f;
}

void EmulatorItem::resetGame() {
  m_renderer->submitCommand({.type = EmulatorItemRenderer::ResetGame});
  update();
}

void EmulatorItem::writeSuspendPoint(const int index) {
  m_renderer->submitCommand({.type = EmulatorItemRenderer::WriteSuspendPoint,
                             .suspendPointIndex = index});
  update();
}

void EmulatorItem::loadSuspendPoint(const int index) {
  m_renderer->submitCommand({.type = EmulatorItemRenderer::LoadSuspendPoint,
                             .suspendPointIndex = index});
  update();
}
void EmulatorItem::undoLastLoadSuspendPoint() {
  m_renderer->submitCommand(
      {.type = EmulatorItemRenderer::UndoLoadSuspendPoint});
  update();
}

void EmulatorItem::createRewindPoints() {
  m_renderer->submitCommand({.type = EmulatorItemRenderer::EmitRewindPoints});
  update();
}

void EmulatorItem::loadRewindPoint(const int index) {
  m_renderer->submitCommand({.type = EmulatorItemRenderer::LoadRewindPoint,
                             .rewindPointIndex = index});
  update();
}

void EmulatorItem::setPlaybackMultiplier(int playbackMultiplier) {
  if (m_playbackMultiplier != playbackMultiplier) {
    m_playbackMultiplier = playbackMultiplier;
    emit playbackMultiplierChanged();

    m_renderer->submitCommand(
        {.type = EmulatorItemRenderer::SetPlaybackMultiplier,
         .playbackMultiplier = m_playbackMultiplier});
    update();
  }
}

void EmulatorItem::hoverMoveEvent(QHoverEvent *event) {
  const auto pos = event->position();
  const auto bounds = boundingRect();

  const auto x = (pos.x() - bounds.width() / 2) / (bounds.width() / 2);
  const auto y = (pos.y() - bounds.height() / 2) / (bounds.height() / 2);

  getInputManager()->updateMouseState(x, y, m_mousePressed);
}

void EmulatorItem::mousePressEvent(QMouseEvent *event) {
  m_mousePressed = true;
  getInputManager()->updateMousePressed(m_mousePressed);
}

void EmulatorItem::mouseReleaseEvent(QMouseEvent *event) {
  m_mousePressed = false;
  getInputManager()->updateMousePressed(m_mousePressed);
}
void EmulatorItem::loadGame(int entryId) {
  m_threadPool.start([this, entryId] {
    spdlog::info("Loading entry with id {}", entryId);

    auto entry = getUserLibrary()->getEntry(entryId);

    // auto entry = getLibraryDatabase()->getLibraryEntry(entryId);
    //
    if (!entry.has_value()) {
      spdlog::warn("Entry with id {} does not exist...", entryId);
    }

    m_gameName = entry->displayName;
    //
    // m_currentEntry = *entry;

    auto roms =
        getUserLibrary()->getRomFilesWithContentHash(entry->contentHash);
    if (roms.empty()) {
      spdlog::warn("No ROM file found for entry with id {}", entryId);
    }

    auto romFile = std::unique_ptr<firelight::library::RomFile>();
    for (auto &rom : roms) {
      if (!rom.inArchive()) {
        romFile = std::make_unique<firelight::library::RomFile>(rom);
      }
    }

    // If we didn't find one that isn't in an archive, just use the first one
    if (!romFile) {
      romFile = std::make_unique<firelight::library::RomFile>(roms[0]);
    }

    if (romFile->inArchive() &&
        !std::filesystem::exists(romFile->getArchivePathName().toStdString())) {
      spdlog::error("Content path doesn't exist: {}",
                    romFile->getArchivePathName().toStdString());
      return;
    }

    if (!romFile->inArchive() &&
        !std::filesystem::exists(romFile->getFilePath().toStdString())) {
      spdlog::error("Content path doesn't exist: {}",
                    romFile->getFilePath().toStdString());
      return;
    }

    romFile->load();

    std::string corePath =
        firelight::PlatformMetadata::getCoreDllPath(entry->platformId);
    //
    QByteArray saveDataBytes;
    const auto saveData = getSaveManager()->readSaveData(
        romFile->getContentHash(), entry->activeSaveSlot);
    if (saveData.has_value()) {
      saveDataBytes = QByteArray(saveData->getSaveRamData().data(),
                                 saveData->getSaveRamData().size());
    }

    m_entryId = entryId;
    m_gameData = romFile->getContentBytes();
    m_saveData = saveDataBytes;
    m_corePath = QString::fromStdString(corePath);
    m_contentHash = romFile->getContentHash();
    m_saveSlotNumber = entry->activeSaveSlot;
    m_platformId = entry->platformId;
    m_contentPath = romFile->getFilePath();

    m_loaded = true;
    if (m_startAfterLoading) {
      startGame();
    }
  });
}
void EmulatorItem::startGame() {
  if (!m_loaded) {
    m_startAfterLoading = true;
  }

  QThreadPool::globalInstance()->start([this] {
    auto configProvider = getEmulatorConfigManager()->getCoreConfigFor(
        m_platformId, m_contentHash);
    auto m_core = std::make_unique<libretro::Core>(
        m_platformId, m_corePath.toStdString(), configProvider,
        getCoreSystemDirectory());

    m_audioManager = std::make_shared<AudioManager>(
        [this] { emit audioBufferLevelChanged(); });
    m_core->setAudioReceiver(m_audioManager);
    m_core->setRetropadProvider(getControllerManager());
    m_core->setPointerInputProvider(getInputManager());
    m_core->setSystemDirectory(getCoreSystemDirectory());

    // Qt owns the renderer, so it will destoy it.
    m_renderer = new EmulatorItemRenderer(
        window()->rendererInterface()->graphicsApi(), std::move(m_core));

    m_renderer->onGeometryChanged([this](unsigned int width,
                                         unsigned int height, float aspectRatio,
                                         double framerate) {
      updateGeometry(width, height, aspectRatio);
    });

    // m_core->init();

    // Setting these causes the item's geometry to be visible, and the renderer
    // is initialized. If an item is not visible, the renderer is not
    // initialized.
    m_coreBaseWidth = 1;
    m_coreBaseHeight = 1;
    m_calculatedAspectRatio = 0.000001;
    m_coreAspectRatio = 0.000001;

    emit videoWidthChanged();
    emit videoHeightChanged();
    emit videoAspectRatioChanged();

    m_started = true;
    emit gameStarted();
  });
}

QQuickRhiItemRenderer *EmulatorItem::createRenderer() {
  // const auto renderer = new
  // EmulatorItemRenderer(window()->rendererInterface()->graphicsApi(),
  //                                                m_core.get());
  //
  // renderer->onGeometryChanged([this](int width, int height, float
  // aspectRatio) {
  //     updateGeometry(width, height, aspectRatio);
  // });

  return m_renderer;
}

void EmulatorItem::updateGeometry(unsigned int width, unsigned int height,
                                  float aspectRatio) {
  m_coreBaseWidth = width;
  m_coreBaseHeight = height;
  m_coreAspectRatio = aspectRatio;
  m_calculatedAspectRatio = static_cast<float>(m_coreBaseWidth) /
                            static_cast<float>(m_coreBaseHeight);

  spdlog::info(
      "width: {}, height: {}, aspectRatio: {}, calculatedAspectRatio: {}",
      m_coreBaseWidth, m_coreBaseHeight, m_coreAspectRatio,
      m_calculatedAspectRatio);
  setFixedColorBufferWidth(m_coreBaseWidth);
  setFixedColorBufferHeight(m_coreBaseHeight);
  emit videoWidthChanged();
  emit videoHeightChanged();
  emit videoAspectRatioChanged();
}

//
// Created by alexs on 12/20/2023.
//

#include "emulation_manager.hpp"
#include "../gui/controller_manager.hpp"
#include <QGuiApplication>
#include <QOpenGLPaintDevice>
#include <QPainter>
#include <QSGImageNode>
#include <QSGTexture>
#include <qopenglcontext.h>

#include "audio_manager.hpp"

#include <spdlog/spdlog.h>

EmulationManager *instance;

constexpr int SAVE_FREQUENCY_MILLIS = 10000;

QSGNode *
EmulationManager::updatePaintNode(QSGNode *qsg_node,
                                  UpdatePaintNodeData *update_paint_node_data) {
  if (!gameTexture) {
    update();
    return nullptr;
  }

  auto texNode = dynamic_cast<QSGImageNode *>(qsg_node);
  if (!texNode) {
    texNode = window()->createImageNode();
  }

  texNode->setTexture(gameTexture);
  texNode->setTextureCoordinatesTransform(QSGImageNode::MirrorVertically);
  texNode->setRect(boundingRect());

  return texNode;
}

EmulationManager *EmulationManager::getInstance() {
  if (!instance) {
    instance = new EmulationManager();
  }

  return instance;
}

void EmulationManager::setLibraryManager(QLibraryManager *manager) {
  library_manager_ = manager;
}

EmulationManager::EmulationManager(QQuickItem *parent) : QQuickItem(parent) {}

void EmulationManager::registerInstance(EmulationManager *manager) {}

void EmulationManager::initialize(int entryId) {
  printf("initializing\n");
  auto entry = library_manager_->get_by_id(entryId);

  if (!entry.has_value()) {
    printf("OH NOOOOO no entry with id %d\n", entryId);
  }

  m_currentEntry = entry.value();

  std::string corePath;
  if (entry->platform == 0) {
    corePath = "./system/_cores/mupen64plus_next_libretro.dll";
  } else if (entry->platform == 1) {
    corePath = "./system/_cores/snes9x_libretro.dll";
  }

  core = std::make_unique<libretro::Core>(corePath);
  core->setRetropadProvider(getControllerManager());

  core->setSystemDirectory(".");
  core->setSaveDirectory(".");

  core->set_video_receiver(this);
  core->set_audio_receiver(new AudioManager());
  core->init();

  libretro::Game game(entry->content_path);
  core->loadGame(&game);

  const auto saveData = getSaveManager()->readSaveDataForEntry(*entry);
  if (saveData.has_value()) {
    if (saveData->getSaveRamData().empty()) {
      spdlog::warn("Save data was present but there are no bytes");
    } else {
      core->writeMemoryData(libretro::SAVE_RAM, saveData->getSaveRamData());
    }
  }

  window()->setMinimumSize(QSize(core_av_info_->geometry.max_width,
                                 core_av_info_->geometry.max_height));

  setSize(QSize(core_av_info_->geometry.max_width,
                core_av_info_->geometry.max_height));

  const auto targetFrameTime = 1 / core_av_info_->timing.fps;
  const auto actualFrameTime =
      1 / QGuiApplication::primaryScreen()->refreshRate();

  frameSkipRatio = std::lround(targetFrameTime / actualFrameTime);
  printf("setting frame skip ratio to %d\n", frameSkipRatio);

  setFlag(ItemHasContents);
  auto win = window();

  connect(win, &QQuickWindow::beforeRenderPassRecording, this,
          &EmulationManager::runOneFrame, Qt::DirectConnection);
}

void EmulationManager::runOneFrame() {
  if (!glInitialized) {
    initializeOpenGLFunctions();
    glInitialized = true;
  }

  if (running) {
    if (reset_context) {
      usingHwRendering = true;

      gameFbo = std::make_unique<QOpenGLFramebufferObject>(
          core_av_info_->geometry.max_width,
          core_av_info_->geometry.max_height);
      gameFbo->setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);

      gameTexture = QNativeInterface::QSGOpenGLTexture::fromNative(
          gameFbo->texture(), window(), gameFbo->size());

      if (!gameFbo->isValid()) {
        printf("gameFbo is not valid :(\n");
      }

      reset_context();
      reset_context = nullptr;
    }

    auto frameBegin = SDL_GetPerformanceCounter();
    lastTick = thisTick;
    thisTick = SDL_GetPerformanceCounter();

    auto deltaTime =
        (thisTick - lastTick) * 1000 / (double)SDL_GetPerformanceFrequency();

    m_millisSinceLastSave += static_cast<int>(deltaTime);
    if (m_millisSinceLastSave < 0) {
      m_millisSinceLastSave = 0;
    }

    if (m_millisSinceLastSave >= SAVE_FREQUENCY_MILLIS) {
      m_millisSinceLastSave = 0;
      Firelight::Saves::SaveData saveData(
          core->getMemoryData(libretro::SAVE_RAM));
      saveData.setImage(gameFbo->toImage());

      getSaveManager()->writeSaveDataForEntry(m_currentEntry, saveData);
    }

    window()->beginExternalCommands();

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    frameCount++;
    if (frameSkipRatio == 0 || (frameCount % frameSkipRatio == 0)) {
      core->run(deltaTime);

      auto frameEnd = SDL_GetPerformanceCounter();
      auto frameDiff = ((frameEnd - frameBegin) * 1000 /
                        static_cast<double>(SDL_GetPerformanceFrequency()));
      totalFrameWorkDurationMillis += frameDiff;
      numFrames++;

      if (numFrames == 300) {
        printf("Average frame work duration: %fms\n",
               totalFrameWorkDurationMillis / numFrames);
        totalFrameWorkDurationMillis = 0;
        numFrames = 0;
      }
    }

    window()->endExternalCommands();
    window()->update();
  }
}
void EmulationManager::pause() { running = false; }
void EmulationManager::resume() { running = true; }

void EmulationManager::receive(const void *data, unsigned int width,
                               unsigned int height, size_t pitch) {
  if (data != nullptr && !usingHwRendering) {
    if (!gameFbo) {
      gameFbo = std::make_unique<QOpenGLFramebufferObject>(width, height);

      gameTexture = QNativeInterface::QSGOpenGLTexture::fromNative(
          gameFbo->texture(), window(), gameFbo->size());
    }

    QOpenGLPaintDevice paint_device;
    paint_device.setSize(gameFbo->size());
    QPainter painter(&paint_device);

    gameFbo->bind();
    const QImage image((uchar *)data, width, height, pitch,
                       QImage::Format_RGB16);

    painter.drawImage(QRect(0, 0, gameFbo->width(), gameFbo->height()), image,
                      image.rect());
    gameFbo->release();
  }
}

proc_address_t EmulationManager::get_proc_address(const char *sym) {
  return QOpenGLContext::currentContext()->getProcAddress(sym);
}

void EmulationManager::set_system_av_info(retro_system_av_info *info) {
  core_av_info_ = info;
}

void EmulationManager::set_reset_context_func(context_reset_func reset) {
  reset_context = reset;
}

uintptr_t EmulationManager::get_current_framebuffer_id() {
  return gameFbo->handle();
}

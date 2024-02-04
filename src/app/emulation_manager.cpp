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
#include "emulator_renderer.hpp"

#include <spdlog/spdlog.h>

EmulationManager *instance;

constexpr int SAVE_FREQUENCY_MILLIS = 10000;

QQuickFramebufferObject::Renderer *EmulationManager::createRenderer() const {
  return new EmulatorRenderer(m_entryId, m_gameData, m_saveData, m_corePath);
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

EmulationManager::EmulationManager(QQuickItem *parent)
    : QQuickFramebufferObject(parent) {
  setTextureFollowsItemSize(false);
  setMirrorVertically(true);
  setFlag(ItemHasContents);
}
EmulationManager::~EmulationManager() {
  running = false;
  // save(true);
  // if (m_renderConnection) {
  //   disconnect(m_renderConnection);
  // }
}

void EmulationManager::registerInstance(EmulationManager *manager) {}

void EmulationManager::load(int entryId, const QByteArray &gameData,
                            const QByteArray &saveData,
                            const QString &corePath) {
  m_entryId = entryId;
  m_gameData = gameData;
  m_saveData = saveData;
  m_corePath = corePath;
}

void EmulationManager::runOneFrame() {
  if (!glInitialized) {
    window()->beginExternalCommands();
    initializeOpenGLFunctions();
    glInitialized = true;
    window()->endExternalCommands();
  }

  if (running) {
    window()->beginExternalCommands();
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
      gameImage = gameFbo->toImage();
      // save();
    }

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    frameCount++;
    if (frameSkipRatio == 0 || (frameCount % frameSkipRatio == 0)) {
      // glEnable(GL_BLEND);
      // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      // gameFbo->bind();
      core->run(deltaTime);
      // gameFbo->release();
      // glDisable(GL_BLEND);

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

void EmulationManager::setPaused(bool paused) {
  running = !paused;
  update();
}

bool EmulationManager::paused() const { return !running; }

// void EmulationManager::receive(const void *data, unsigned int width,
//                                unsigned int height, size_t pitch) {
//   if (data != nullptr && !usingHwRendering) {
//     if (!gameFbo) {
//       gameFbo = std::make_unique<QOpenGLFramebufferObject>(width, height);
//
//       gameTexture = QNativeInterface::QSGOpenGLTexture::fromNative(
//           gameFbo->texture(), window(), gameFbo->size());
//     }
//
//     QOpenGLPaintDevice paint_device;
//     paint_device.setSize(gameFbo->size());
//     QPainter painter(&paint_device);
//
//     gameFbo->bind();
//     const QImage image((uchar *)data, width, height, pitch,
//                        QImage::Format_RGB16);
//
//     painter.drawImage(QRect(0, 0, gameFbo->width(), gameFbo->height()),
//     image,
//                       image.rect());
//     gameFbo->release();
//   }
// }

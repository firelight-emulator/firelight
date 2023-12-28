//
// Created by alexs on 12/20/2023.
//

#include "emulation_manager.hpp"
#include <QGuiApplication>
#include <QOpenGLPaintDevice>
#include <QPainter>
#include <QSGImageNode>
#include <QSGTexture>
#include <SDL_gamecontroller.h>
#include <qopenglcontext.h>

EmulationManager *instance;

void EmulationManager::set_system_av_info(retro_system_av_info *info) {
  printf("got system av info fps: %f, sample rate: %f\n", info->timing.fps,
         info->timing.sample_rate);
  core_av_info_ = info;
}

QSGNode *
EmulationManager::updatePaintNode(QSGNode *qsg_node,
                                  UpdatePaintNodeData *update_paint_node_data) {
  printf("updating paint node\n");
  if (!gameTexture) {
    update();
    return nullptr;
  }

  auto texNode = dynamic_cast<QSGImageNode *>(qsg_node);
  if (!texNode) {
    texNode = window()->createImageNode();
  }

  // if (usingHwRendering) {
  //   printf("setting HARDWARE tex!");
  // } else {
  //   texNode->setTexture(gameTexture);
  // }
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

void EmulationManager::setLibraryManager(FL::Library::LibraryManager *manager) {
  library_manager_ = manager;
}

EmulationManager::EmulationManager(QQuickItem *parent) : QQuickItem(parent) {}

void EmulationManager::registerInstance(EmulationManager *manager) {}

void EmulationManager::initialize(int entryId) {
  printf("initializing\n");
  SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");
  conManager.scanGamepads();

  auto entry = library_manager_->getEntryById(entryId);

  if (!entry.has_value()) {
    printf("OH NOOOOO no entry with id %d\n", entryId);
  }

  std::string corePath;
  if (entry->platform == "n64") {
    corePath = "./system/_cores/mupen64plus_next_libretro.dll";
  } else if (entry->platform == "snes") {
    corePath = "./system/_cores/snes9x_libretro.dll";
  }

  core = std::make_unique<libretro::Core>(corePath, &conManager);

  core->setSystemDirectory(".");
  core->setSaveDirectory(".");

  core->setVideoStuff(this);
  core->init();

  libretro::Game game(entry->content_path);
  core->loadGame(&game);
  window()->setMinimumSize(QSize(core_av_info_->geometry.max_width,
                                 core_av_info_->geometry.max_height));

  setSize(QSize(core_av_info_->geometry.max_width,
                core_av_info_->geometry.max_height));

  auto targetFrameTime = 1 / core_av_info_->timing.fps;
  auto actualFrameTime = 1 / QGuiApplication::primaryScreen()->refreshRate();

  frameSkipRatio = std::floor(targetFrameTime / actualFrameTime);
  printf("setting frame skip ratio to %d", frameSkipRatio);

  // const double refresh = QGuiApplication::primaryScreen()->refreshRate();
  // std::printf("Refresh Rate: %f \r\n", refresh);
  // if (refresh > 61) {
  //   numSkipFrames = ((refresh / 60.0) + 0.5) - 1.0;
  //   std::printf("Number of skipped frames: %d \r\n", numSkipFrames);
  // }

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

    frameCount++;
    if (frameSkipRatio != 0 && (frameCount % frameSkipRatio != 0)) {
      window()->update();
      return;
    }

    // if (currentSkippedFrames == numSkipFrames && numSkipFrames != 0) {
    //   currentSkippedFrames = 0;
    //   window()->update();
    //   return;
    // }
    // currentSkippedFrames++;

    frameBegin = SDL_GetPerformanceCounter();
    lastTick = thisTick;
    thisTick = SDL_GetPerformanceCounter();

    auto deltaTime =
        (thisTick - lastTick) * 1000 / (double)SDL_GetPerformanceFrequency();

    window()->beginExternalCommands();

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    core->run(deltaTime);

    window()->endExternalCommands();

    frameEnd = SDL_GetPerformanceCounter();
    frameDiff = ((frameEnd - frameBegin) * 1000 /
                 static_cast<double>(SDL_GetPerformanceFrequency()));
    totalFrameWorkDurationMillis += frameDiff;
    numFrames++;

    if (numFrames == 300) {
      printf("Average frame work duration: %fms\n",
             totalFrameWorkDurationMillis / numFrames);
      totalFrameWorkDurationMillis = 0;
      numFrames = 0;
    }
    window()->update();
  }

  // window()->beginExternalCommands();

  // if (gameFbo) {
  //   QOpenGLFramebufferObject::blitFramebuffer(
  //       nullptr, QRect(x(), y(), width(), height()), gameFbo.get(),
  //       boundingRect().toRect(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
  //       GL_NEAREST, GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT0);
  // }
  // window()->endExternalCommands();
}
void EmulationManager::pause() { running = false; }
void EmulationManager::resume() { running = true; }

void EmulationManager::receive(const void *data, unsigned int width,
                               unsigned int height, size_t pitch) {
  if (data != nullptr && !usingHwRendering) {
    if (!gameFbo) {
      printf("creating fbo and texture\n");
      gameFbo = std::make_unique<QOpenGLFramebufferObject>(width, height);

      gameTexture = QNativeInterface::QSGOpenGLTexture::fromNative(
          gameFbo->texture(), window(), gameFbo->size());
    }

    QOpenGLPaintDevice paint_device;
    paint_device.setSize(gameFbo->size());
    QPainter painter(&paint_device);

    gameFbo->bind();
    QImage image((uchar *)data, width, height, pitch, QImage::Format_RGB16);

    painter.drawImage(QRect(0, 0, gameFbo->width(), gameFbo->height()), image,
                      image.rect());
    gameFbo->release();
  }
}

proc_address_t EmulationManager::get_proc_address(const char *sym) {
  const auto context = QOpenGLContext::currentContext();
  auto addr = context->getProcAddress(sym);
  return addr;
}

void EmulationManager::set_reset_context_func(context_reset_func reset) {
  reset_context = reset;
}

uintptr_t EmulationManager::get_current_framebuffer_id() {
  return gameFbo->handle();
}

std::unique_ptr<QImage> EmulationManager::getImage() {
  return nullptr;
  auto image = std::make_unique<QImage>((uchar *)m_data, m_width, m_height,
                                        m_pitch, QImage::Format_RGB16);

  image->mirror();
  return image;
}

//
// Created by alexs on 12/20/2023.
//

#include "emulation_manager.hpp"

#include <QGuiApplication>
#include <QSGSimpleTextureNode>
#include <SDL_gamecontroller.h>
#include <qopenglcontext.h>

EmulationManager *instance;

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

void EmulationManager::initialize() {
  printf("initializing\n");
  SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");
  conManager.scanGamepads();

  if (current_lib_entry_id_ == -1) {
    printf("oh noooooo\n");
  }

  auto entry = library_manager_->getEntryById(current_lib_entry_id_);

  if (!entry.has_value()) {
    printf("OH NOOOOO no entry with id %d\n", current_lib_entry_id_);
  }

  std::string corePath;
  if (entry->platform == "n64") {
    corePath = "./system/_cores/mupen64plus_next_libretro.dll";
  } else {
    printf("ok\n");
  }

  core = std::make_unique<libretro::Core>(corePath, &conManager);

  core->setSystemDirectory(".");
  core->setSaveDirectory(".");

  core->setVideoStuff(this);
  core->init();

  libretro::Game game(entry->content_path);
  core->loadGame(&game);

  const double refresh = QGuiApplication::primaryScreen()->refreshRate();
  std::printf("Refresh Rate: %f \r\n", refresh);
  if(refresh > 61) {
    numSkipFrames = ((refresh / 60.0) + 0.5) - 1.0;
    std::printf("Number of skipped frames: %d \r\n", numSkipFrames);
  }

  // setFlag(ItemHasContents);
  auto win = window();

  connect(win, &QQuickWindow::beforeRenderPassRecording, this,
          &EmulationManager::runOneFrame, Qt::DirectConnection);
}

void EmulationManager::loadLibraryEntry(int entryId) {
  current_lib_entry_id_ = entryId;
  printf("Setting library entry: %d\n", current_lib_entry_id_);
}

void EmulationManager::runOneFrame() {
  if (running) {
    if (reset_context) {
      initializeOpenGLFunctions();

      gameFbo = std::make_unique<QOpenGLFramebufferObject>(640, 480);
      gameFbo->setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);

      if (!gameFbo->isValid()) {
        printf("gameFbo is not valid :(\n");
      }

      reset_context();
      reset_context = nullptr;
    }

    if(currentSkippedFrames == numSkipFrames && numSkipFrames != 0) {
      currentSkippedFrames = 0;
      window() -> update();
      return;
    }
    currentSkippedFrames++;

    frameBegin = SDL_GetPerformanceCounter();
    lastTick = thisTick;
    thisTick = SDL_GetPerformanceCounter();

    auto deltaTime =
        (thisTick - lastTick) * 1000 / (double)SDL_GetPerformanceFrequency();

    window()->beginExternalCommands();
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    core->run(deltaTime);

    window()->endExternalCommands();

    //    if (deltaTime > 20) {
    //      printf("delta time: %f\n", deltaTime);
    //    }
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

  window()->beginExternalCommands();

  if (gameFbo) {
    QOpenGLFramebufferObject::blitFramebuffer(
        nullptr, QRect(x(), y(), width(), height()), gameFbo.get(),
        boundingRect().toRect(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
        GL_NEAREST, GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT0);
  }
  window()->endExternalCommands();
}
void EmulationManager::pause() { running = false; }
void EmulationManager::resume() { running = true; }

void EmulationManager::receive(const void *data, unsigned int width,
                               unsigned int height, size_t pitch) {
  if (data != nullptr) {
    m_data = const_cast<void *>(data);
    m_width = width;
    m_height = height;
    m_pitch = pitch;
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

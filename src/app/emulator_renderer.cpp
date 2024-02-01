//
// Created by alexs on 2/1/2024.
//

#include "emulator_renderer.hpp"

#include "audio_manager.hpp"

#include <QOpenGLFramebufferObject>

constexpr int SAVE_FREQUENCY_MILLIS = 10000;

void EmulatorRenderer::receive(const void *data, unsigned width,
                               unsigned height, size_t pitch) {}

proc_address_t EmulatorRenderer::get_proc_address(const char *sym) {
  return QOpenGLContext::currentContext()->getProcAddress(sym);
}

void EmulatorRenderer::set_reset_context_func(context_reset_func reset) {
  reset_context = reset;
}

uintptr_t EmulatorRenderer::get_current_framebuffer_id() {
  return m_fbo->handle();
}
void EmulatorRenderer::set_system_av_info(retro_system_av_info *info) {}

QOpenGLFramebufferObject *
EmulatorRenderer::createFramebufferObject(const QSize &size) {
  printf("Creating new fbo: %d, %d\n", 640, 480);
  m_fbo = Renderer::createFramebufferObject(QSize(640, 480));
  return m_fbo;
}

EmulatorRenderer::EmulatorRenderer(int entryId, QByteArray gameData,
                                   QByteArray saveData, QString corePath) {
  initializeOpenGLFunctions();
  m_currentEntry = getLibraryManager()->get_by_id(entryId).value();

  core = std::make_unique<libretro::Core>(corePath.toStdString());
  core->setRetropadProvider(getControllerManager());

  core->setSystemDirectory(".");
  core->setSaveDirectory(".");

  core->set_video_receiver(this);
  core->set_audio_receiver(new AudioManager());
  core->init();

  libretro::Game game(vector<unsigned char>(gameData.begin(), gameData.end()));
  core->loadGame(&game);

  if (saveData.size() > 0) {
    core->writeMemoryData(libretro::SAVE_RAM,
                          vector(saveData.begin(), saveData.end()));
  }

  // window()->setMinimumSize(QSize(core_av_info_->geometry.max_width,
  //                                core_av_info_->geometry.max_height));

  // setSize(QSize(core_av_info_->geometry.max_width,
  //               core_av_info_->geometry.max_height));

  // const auto targetFrameTime = 1 / core_av_info_->timing.fps;
  // const auto actualFrameTime =
  //     1 / QGuiApplication::primaryScreen()->refreshRate();

  // frameSkipRatio = std::lround(targetFrameTime / actualFrameTime);
}

void EmulatorRenderer::render() {
  // printf("Frame #: %lld\n", frameCount);
  // frameCount++;
  // if (frameCount % 2 == 0) {
  //   glClearColor(1, 0, 0, 1);
  // } else {
  //   glClearColor(0, 1, 0, 1);
  // }
  //
  // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  if (reset_context) {
    usingHwRendering = true;

    // gameFbo = std::make_unique<QOpenGLFramebufferObject>(
    //     core_av_info_->geometry.max_width,
    //     core_av_info_->geometry.max_height);
    //
    // gameFbo->setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    //
    // gameTexture = QNativeInterface::QSGOpenGLTexture::fromNative(
    //     gameFbo->texture(), window(), gameFbo->size());
    //
    // if (!gameFbo->isValid()) {
    //   printf("gameFbo is not valid :(\n");
    // }

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
    printf("Pretending to save\n");
    // gameImage = gameFbo->toImage();
    // save();
  }

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

  update();
}
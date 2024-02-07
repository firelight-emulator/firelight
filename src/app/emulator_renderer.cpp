//
// Created by alexs on 2/1/2024.
//

#include "emulator_renderer.hpp"

#include "audio_manager.hpp"
#include "emulation_manager.hpp"

#include <QOpenGLFramebufferObject>
#include <spdlog/spdlog.h>

constexpr int SAVE_FREQUENCY_MILLIS = 10000;

void EmulatorRenderer::synchronize(QQuickFramebufferObject *fbo) {
  const auto manager = reinterpret_cast<EmulationManager *>(fbo);
  if (manager->takeShouldLoadGameFlag()) {
    m_shouldLoadGame = true;
    m_entryId = manager->getEntryId();
    m_gameData = manager->getGameData();
    m_saveData = manager->getSaveData();
    m_corePath = manager->getCorePath();

    printf("%d, %lld, %lld, %s\n", m_entryId, m_gameData.size(),
           m_saveData.size(), m_corePath.toStdString().c_str());
    update();
  }
  if (manager->takeShouldPauseGameFlag()) {
    m_shouldPauseGame = true;
    update();
  }
  if (manager->takeShouldResumeGameFlag()) {
    m_shouldResumeGame = true;
    update();
  }
  if (manager->takeShouldStartEmulationFlag()) {
    m_shouldStartEmulation = true;
    update();
  }
  if (manager->takeShouldStopEmulationFlag()) {
    m_shouldStopEmulation = true;
    update();
  }

  Renderer::synchronize(fbo);
}

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
void EmulatorRenderer::save(bool waitForFinish) {
  spdlog::debug("Autosaving SRAM data (interval {}ms)", SAVE_FREQUENCY_MILLIS);
  Firelight::Saves::SaveData saveData(core->getMemoryData(libretro::SAVE_RAM));
  saveData.setImage(m_fbo->toImage());

  QFuture<bool> result =
      getSaveManager()->writeSaveDataForEntry(m_currentEntry, saveData);

  if (waitForFinish) {
    result.waitForFinished();
  }
}

QOpenGLFramebufferObject *
EmulatorRenderer::createFramebufferObject(const QSize &size) {
  printf("Creating new fbo: %d, %d\n", 640, 480);
  m_fbo = Renderer::createFramebufferObject(QSize(640, 480));
  return m_fbo;
}
EmulatorRenderer::EmulatorRenderer() { initializeOpenGLFunctions(); }

void EmulatorRenderer::render() {
  if (m_shouldLoadGame) {
    m_currentEntry = getLibraryManager()->get_by_id(m_entryId).value();

    core = std::make_unique<libretro::Core>(m_corePath.toStdString());
    core->setRetropadProvider(getControllerManager());

    core->setSystemDirectory(".");
    core->setSaveDirectory(".");

    core->set_video_receiver(this);
    core->set_audio_receiver(new AudioManager());
    core->init();

    libretro::Game game(
        vector<unsigned char>(m_gameData.begin(), m_gameData.end()));
    core->loadGame(&game);

    if (m_saveData.size() > 0) {
      core->writeMemoryData(libretro::SAVE_RAM,
                            vector(m_saveData.begin(), m_saveData.end()));
    }

    // window()->setMinimumSize(QSize(core_av_info_->geometry.max_width,
    //                                core_av_info_->geometry.max_height));

    // setSize(QSize(core_av_info_->geometry.max_width,
    //               core_av_info_->geometry.max_height));

    // const auto targetFrameTime = 1 / core_av_info_->timing.fps;
    // const auto actualFrameTime =
    //     1 / QGuiApplication::primaryScreen()->refreshRate();

    // frameSkipRatio = std::lround(targetFrameTime / actualFrameTime);
    m_paused = false;
    m_shouldLoadGame = false;
  }

  if (m_shouldStartEmulation) {
    m_running = true;
    if (reset_context) {
      usingHwRendering = true;
      reset_context();
      reset_context = nullptr;
    }
    m_shouldStartEmulation = false;
  }

  if (m_shouldStopEmulation) {
    m_running = false;
    m_shouldStopEmulation = false;
  }

  if (m_shouldPauseGame) {
    m_paused = true;
    m_shouldPauseGame = false;
  }

  if (m_shouldResumeGame) {
    m_paused = false;
    m_shouldResumeGame = false;
  }

  if (m_running && !m_paused) {
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
}
//
// Created by alexs on 2/1/2024.
//

#include "emulator_renderer.hpp"

#include "audio_manager.hpp"
#include "emulation_manager.hpp"

#include <QOpenGLFramebufferObject>
#include <QOpenGLPaintDevice>
#include <QPainter>
#include <QSGTextureProvider>
#include <qdatetime.h>
#include <spdlog/spdlog.h>

constexpr int SAVE_FREQUENCY_MILLIS = 10000;

void EmulatorRenderer::synchronize(QQuickFramebufferObject *fbo) {
  const auto manager = reinterpret_cast<EmulationManager *>(fbo);
  manager->setCurrentEntry(m_currentEntry);

  if (manager->takeShouldLoadGameFlag()) {
    m_entryId = manager->getEntryId();
    m_gameData = manager->getGameData();
    m_saveData = manager->getSaveData();
    m_corePath = manager->getCorePath();

    core_av_info_ = nullptr;

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

    printf("%d, %lld, %lld, %s\n", m_entryId, m_gameData.size(),
           m_saveData.size(), m_corePath.toStdString().c_str());
    invalidateFramebufferObject();
    update();
  }
  if (manager->takeShouldPauseGameFlag()) {
    if (!m_paused) {
      sessionDuration += m_playtimeTimer.restart();
    }
    m_paused = true;
    update();
  }
  if (manager->takeShouldResumeGameFlag()) {
    if (m_paused) {
      m_playtimeTimer.restart();
    }
    m_paused = false;
    update();
  }
  if (manager->takeShouldStartEmulationFlag()) {
    if (!m_running) {
      sessionStartTime = QDateTime::currentMSecsSinceEpoch();
      m_shouldStartEmulation = true;
      m_running = true;
      m_paused = false;
      manager->setIsRunning(true);
      update();
      sessionDuration = 0;
      m_playtimeTimer.start();
    }
  }
  if (manager->takeShouldStopEmulationFlag()) {
    if (m_running) {
      sessionEndTime = QDateTime::currentMSecsSinceEpoch();
      if (!m_paused) {
        sessionDuration += m_playtimeTimer.restart();
      } else {
        m_playtimeTimer.restart();
      }
      getUserdataManager()->savePlaySession(m_currentEntry.md5,
                                            sessionStartTime, sessionEndTime,
                                            sessionDuration / 1000);
      m_running = false;
      m_ranLastFrame = false;
      save(true);
      core_av_info_ = nullptr;
      usingHwRendering = false;
      core->unloadGame();
      core->deinit();
      core = nullptr;
      manager->setIsRunning(false);
      update();
    }
  }
  if (manager->takeShouldResetEmulationFlag()) {
    if (core) {
      m_paused = true;
      core->reset();
      update();
    }
  }

  if (core_av_info_ != nullptr) {
    const auto width = core_av_info_->geometry.max_width;
    const auto height = core_av_info_->geometry.max_height;

    if (width > 0 && height > 0) {
      manager->setNativeWidth(width);
      manager->setNativeHeight(height);
      manager->setNativeAspectRatio(static_cast<float>(width) /
                                    static_cast<float>(height));
    }
  }

  Renderer::synchronize(fbo);
}

void EmulatorRenderer::receive(const void *data, unsigned width,
                               unsigned height, size_t pitch) {
  if (!usingHwRendering && data != nullptr && m_fbo) {
    QOpenGLPaintDevice paint_device;
    paint_device.setSize(m_fbo->size());
    QPainter painter(&paint_device);

    m_fbo->bind();
    const QImage image((uchar *)data, width, height, pitch,
                       QImage::Format_RGB16);

    painter.drawImage(QRect(0, 0, m_fbo->width(), m_fbo->height()), image,
                      image.rect());
    m_fbo->release();
  }
}

proc_address_t EmulatorRenderer::get_proc_address(const char *sym) {
  return QOpenGLContext::currentContext()->getProcAddress(sym);
}

void EmulatorRenderer::set_reset_context_func(context_reset_func reset) {
  reset_context = reset;
}

uintptr_t EmulatorRenderer::get_current_framebuffer_id() {
  return m_fbo->handle();
}
void EmulatorRenderer::set_system_av_info(retro_system_av_info *info) {
  core_av_info_ = info;
}

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
  if (core_av_info_ != nullptr) {
    auto width = core_av_info_->geometry.max_width;
    auto height = core_av_info_->geometry.max_height;

    if (width > 0 && height > 0) {
      m_fbo = Renderer::createFramebufferObject(QSize(width, height));
      printf("ACTUAL SIZE: %d, %d\n", width, height);
    }
  } else {
    m_fbo = Renderer::createFramebufferObject(size);
  }

  m_fbo->bind();

  // Set the clear color to black
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  // Clear the framebuffer to black
  glClear(GL_COLOR_BUFFER_BIT);

  // Unbind the framebuffer
  m_fbo->release();

  return m_fbo;
}
EmulatorRenderer::EmulatorRenderer() { initializeOpenGLFunctions(); }
EmulatorRenderer::~EmulatorRenderer() {
  if (m_running) {
    if (!m_paused) {
      sessionDuration += m_playtimeTimer.restart();
    } else {
      m_playtimeTimer.restart();
    }
    printf("session duration: %lld seconds\n", sessionDuration / 1000);
    m_running = false;
    m_ranLastFrame = false;

    if (core) {
      save(true);
      core->unloadGame();
      core->deinit();
      core = nullptr;
    }
    core_av_info_ = nullptr;
    usingHwRendering = false;
  }
}

void EmulatorRenderer::render() {
  if (m_shouldStartEmulation) {
    if (reset_context) {
      usingHwRendering = true;
      reset_context();
      reset_context = nullptr;
    }
    m_shouldStartEmulation = false;
  }

  if (m_running) {
    if (!m_paused) {
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
        // gameImage = gameFbo->toImage();
        save(false);
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
    m_ranLastFrame = true;
  }
}
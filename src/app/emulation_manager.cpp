//
// Created by alexs on 12/20/2023.
//

#include "emulation_manager.hpp"
#include <SDL_gamecontroller.h>

EmulationManager *instance;

EmulationManager *EmulationManager::getInstance() {
  if (!instance) {
    instance = new EmulationManager();
  }

  return instance;
}

void EmulationManager::initialize() {
  SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");
  conManager.scanGamepads();

  core = std::make_unique<libretro::Core>("./system/_cores/snes9x_libretro.dll",
                                          &conManager);

  core->setSystemDirectory(".");
  core->setSaveDirectory(".");

  core->setVideoStuff(this);
  core->init();

  libretro::Game game("./roms/smw.smc");
  core->loadGame(&game);
  running = true;
}

void EmulationManager::runOneFrame() {
  if (running) {
    frameBegin = SDL_GetPerformanceCounter();
    lastTick = thisTick;
    thisTick = SDL_GetPerformanceCounter();

    auto deltaTime =
        ((thisTick - lastTick) * 1000 / (double)SDL_GetPerformanceFrequency());
    //    if (deltaTime > 27 || deltaTime < 10) {
    //      printf("delta time: %f\n", deltaTime);
    //    }

    //    printf("delta time: %f\n", deltaTime);
    //    if (dumb == 0) {
    core->run(deltaTime);
    //    if (deltaTime > 20) {
    //      printf("delta time: %f\n", deltaTime);
    //    }
    frameEnd = SDL_GetPerformanceCounter();
    frameDiff = ((frameEnd - frameBegin) * 1000 /
                 (double)SDL_GetPerformanceFrequency());
    totalFrameWorkDurationMillis += frameDiff;
    numFrames++;

    if (numFrames == 300) {
      //      printf("Average frame work duration: %fms\n",
      //             totalFrameWorkDurationMillis / numFrames);
      totalFrameWorkDurationMillis = 0;
      numFrames = 0;
    }

    if (frameDiff > 3) {
      printf("frame diff: %f\n", frameDiff);
    }
    //      dumb++;
    //    } else {
    //      dumb = 0;
    //    }
  }
}

void EmulationManager::receive(const void *data, unsigned int width,
                               unsigned int height, size_t pitch) {
  m_data = const_cast<void *>(data);
  m_width = width;
  m_height = height;
  m_pitch = pitch;
}

std::unique_ptr<QImage> EmulationManager::getImage() {
  auto image = std::make_unique<QImage>((uchar *)m_data, m_width, m_height,
                                        m_pitch, QImage::Format_RGB16);

  image->mirror();
  return image;
}

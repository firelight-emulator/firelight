//
// Created by alexs on 12/20/2023.
//

#include "emulation_manager.hpp"
#include <SDL_gamecontroller.h>
#include <qopenglcontext.h>

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

  core = std::make_unique<libretro::Core>(
      "./system/_cores/mupen64plus_next_libretro.dll", &conManager);

  core->setSystemDirectory(".");
  core->setSaveDirectory(".");

  core->setVideoStuff(this);
  core->init();

  libretro::Game game("./roms/papermario.z64");
  core->loadGame(&game);

  running = true;

  // GLuint otherTex;
  // glGenTextures(1, &otherTex);
  // glBindTexture(GL_TEXTURE_2D, otherTex);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1280, 720, 0, GL_RGBA,
  //              GL_UNSIGNED_BYTE, nullptr);
  //
  // GLuint framebufferobject;
  // glGenFramebuffers(1, &framebufferobject);
  // glBindFramebuffer(GL_FRAMEBUFFER, framebufferobject);
  // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
  //                        otherTex, 0);
  //
  // GLuint renderBuffer;
  // glGenRenderbuffers(1, &renderBuffer);
  // glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
  // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 1280, 720);
  // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
  //                           GL_RENDERBUFFER, renderBuffer);
  //
  // auto s = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  // if (s != GL_FRAMEBUFFER_COMPLETE) {
  //   printf("frame buffer not complete %d\n", s);
  // }
}

void EmulationManager::runOneFrame() {
  if (running) {
    if (reset_context) {
      reset_context();
      reset_context = nullptr;
    }

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

proc_address_t EmulationManager::get_proc_address(const char *sym) {
  const auto context = QOpenGLContext::currentContext();
  auto addr = context->getProcAddress(sym);
  return addr;
}

void EmulationManager::set_reset_context_func(context_reset_func reset) {
  reset_context = reset;
}
uintptr_t EmulationManager::get_current_framebuffer_id() {
  return framebufferProvider->get_framebuffer_handle();
}

void EmulationManager::set_framebuffer_thing(
    FramebufferHandleProvider *provider) {
  framebufferProvider = provider;
}

std::unique_ptr<QImage> EmulationManager::getImage() {
  return nullptr;
  auto image = std::make_unique<QImage>((uchar *)m_data, m_width, m_height,
                                        m_pitch, QImage::Format_RGB16);

  image->mirror();
  return image;
}

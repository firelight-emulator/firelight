//
// Created by alexs on 12/20/2023.
//

#include "emulator_renderer.hpp"
uintptr_t EmulatorRenderer::get_framebuffer_handle() {
  return framebufferobject;
}
EmulatorRenderer::EmulatorRenderer(QQuickWindow *window) : m_window(window) {
  printf("Creating emulator renderer\n");
  EmulationManager::getInstance()->set_framebuffer_thing(this);
}

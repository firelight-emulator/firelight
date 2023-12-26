//
// Created by alexs on 12/20/2023.
//

#include "emulator_view.hpp"
#include "emulator_renderer.hpp"
#include "src/app/emulation_manager.hpp"
#include <QCoreApplication>
#include <QGuiApplication>
#include <QQuickGraphicsConfiguration>

EmulatorView::EmulatorView(QQuickItem *parent)
    : QQuickFramebufferObject(parent) {
  printf("Creating EmulatorView\n");
}

QQuickFramebufferObject::Renderer *EmulatorView::createRenderer() const {
  printf("Creating EmulatorViewRenderer\n");
  return new EmulatorRenderer(window());
}
void EmulatorView::init() {
  EmulationManager::getInstance()->initialize();
  printf("called init\n");
  // window()->update();
}
void EmulatorView::updateThing() { window()->update(); }

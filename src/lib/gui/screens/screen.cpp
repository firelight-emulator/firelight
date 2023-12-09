//
// Created by alexs on 11/13/2023.
//

#include "screen.hpp"

namespace FL::GUI {

Screen::Screen(std::unique_ptr<ContainerWidget> container)
    : mainContainer(std::move(container)) {}

void Screen::enter() {}
void Screen::exit() {}
void Screen::update(float deltaTime) {}
void Screen::render(const std::shared_ptr<WidgetPainter> &painter) {
  mainContainer->paint(painter.get(), FL::Math::Box(0, 0, 1280, 720));
}

void Screen::addWidget(std::unique_ptr<Widget> widget) {
  widget->onFocusGained.connect([this](Widget *w) {
    if (focusTarget && focusTarget != w) {
      focusTarget->loseFocus();
    }
    focusTarget = w;
  });

  widget->onFocusLost.connect([this](Widget *w) {
    if (focusTarget == w) {
      focusTarget = nullptr;
    }
  });

  auto rawWidgetPtr = widget.get();

  mainContainer->addChild(std::move(widget));

  if (!focusTarget) {
    rawWidgetPtr->gainFocus();
  }
}

bool Screen::handleEvent(Event &event) {
  if (focusTarget) {
    return focusTarget->handleEvent(event);
  }

  return false;
}

void Screen::setWindowArea(int width, int height) {
  windowWidth = width;
  windowHeight = height;
}

void Screen::forceStop() {}

} // namespace FL::GUI
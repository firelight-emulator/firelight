//
// Created by alexs on 11/9/2023.
//

#include "button.hpp"

#include <sstream>
#include <utility>

namespace FL::GUI {

Button::Button(std::string text) : label(std::move(text)) {}

void Button::paint(WidgetPainter *painter, FL::Math::Box box) {
  auto pos = getAbsolutePosition();
  FL::Math::Box cool(pos.x, pos.y, this->box.widthPx, this->box.heightPx);

  if (isFocused) {
    painter->paintBackground(cool, focusStyle);
    painter->paintText(label, cool, focusStyle);
  } else {
    painter->paintText(label, cool, inactiveStyle);
  }
}

bool Button::focusable() { return true; }
Widget *Button::getFirstFocusable() { return this; }

bool Button::handleEvent(Event &event) {
  if (event.type == Event::NAV_SELECT_PUSHED) {
    onPressed.emit(this);
  }

  return Widget::handleEvent(event);
}

} // namespace FL::GUI

//
// Created by alexs on 11/7/2023.
//

#include "widget.hpp"

namespace FL::GUI {

void Widget::setStyle(Style *widgetStyle) { style = widgetStyle; }

void Widget::setParent(Widget *newParent) { parent = newParent; }
Widget *Widget::getParent() const { return parent; }

bool Widget::handleEvent(Event &event) {
  if (parent && parent->handleEvent(event)) {
    return true;
  }

  return false;
}

void Widget::setId(int widgetId) { id = widgetId; }

bool Widget::gainFocus() {
  isFocused = true;
  onFocusGained.emit(this);
  return true;
}

void Widget::loseFocus() {
  isFocused = false;
  onFocusLost.emit(this);
}

} // namespace FL::GUI

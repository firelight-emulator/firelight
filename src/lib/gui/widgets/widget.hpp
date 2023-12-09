//
// Created by alexs on 11/7/2023.
//

#ifndef FIRELIGHT_WIDGET_HPP
#define FIRELIGHT_WIDGET_HPP

#include "../../math/bbox.hpp"
#include "../../math/position.hpp"
#include "../events/event.hpp"
#include "../signal.hpp"
#include "../widget_painter.hpp"
#include <functional>
#include <unordered_map>
#include <vector>

namespace FL::GUI {

class Widget {

public:
  virtual ~Widget() = default;
  virtual void paint(WidgetPainter *painter, FL::Math::Box box) = 0;
  virtual bool focusable() = 0;

  virtual Widget *getFirstFocusable() = 0;

  virtual bool isContainer() const { return false; }

  void setStyle(Style *style);
  void setParent(Widget *newParent);
  Widget *getParent() const;
  virtual bool handleEvent(FL::GUI::Event &event);

  bool isFocused = false;

  virtual void show() { isVisible = true; }
  virtual void hide() { isVisible = false; }

  [[nodiscard]] FL::Math::Box getLayout() const { return box; }

  FL::Math::Box box{};

  int getId() const { return id; }

  virtual FL::Math::Position getAbsolutePosition() {
    if (parent) {
      auto parentPos = parent->getAbsolutePosition();
      return FL::Math::Position{parentPos.x + box.xPx, parentPos.y + box.yPx};
    }

    return FL::Math::Position{box.xPx, box.yPx};
  }

  Signal<Widget *> onFocusGained;
  Signal<Widget *> onFocusLost;

  virtual bool gainFocus();
  virtual void loseFocus();

protected:
  int id = -1;

  friend class WidgetFactory;
  void setId(int id);

  Widget *neighborUp = nullptr;
  Widget *neighborDown = nullptr;
  Widget *neighborLeft = nullptr;
  Widget *neighborRight = nullptr;

  Widget *parent = nullptr;
  Style *style = nullptr;

  // Visibility and focus flags
  bool isVisible = true;
};

} // namespace FL::GUI

#endif // FIRELIGHT_WIDGET_HPP

//
// Created by alexs on 11/9/2023.
//

#ifndef FIRELIGHT_BUTTON_HPP
#define FIRELIGHT_BUTTON_HPP

#include "widget.hpp"
#include <functional>
namespace FL::GUI {

class Button : public Widget {
public:
  explicit Button(std::string text);
  void paint(WidgetPainter *painter, FL::Math::Box box) override;
  Widget *getFirstFocusable() override;
  bool focusable() override;
  bool handleEvent(Event &event) override;

  Signal<Button *> onPressed;

private:
  std::string label;
};

} // namespace FL::GUI

#endif // FIRELIGHT_BUTTON_HPP

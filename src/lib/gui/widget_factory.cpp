//
// Created by alexs on 11/12/2023.
//

#include "widget_factory.hpp"
#include "../../app/gui/themes.hpp"

namespace FL::GUI {

std::unique_ptr<Button> WidgetFactory::createButton(const std::string &label) {
  auto button = std::make_unique<Button>(label);
  button->setId(getNextId());
  button->focusStyle = FL::GUI::Themes::buttonFocus;
  button->activeStyle = FL::GUI::Themes::buttonFocus;
  button->inactiveStyle = FL::GUI::Themes::buttonInactive;

  return button;
}
int WidgetFactory::getNextId() { return numWidgets++; }

} // namespace FL::GUI

//
// Created by alexs on 11/12/2023.
//

#ifndef FIRELIGHT_WIDGET_FACTORY_HPP
#define FIRELIGHT_WIDGET_FACTORY_HPP

#include "widgets/button.hpp"
#include <memory>

namespace FL::GUI {

class WidgetFactory {
public:
  std::unique_ptr<Button> createButton(const std::string &label);

private:
  int numWidgets = 0;
  int getNextId();
};

} // namespace FL::GUI

#endif // FIRELIGHT_WIDGET_FACTORY_HPP

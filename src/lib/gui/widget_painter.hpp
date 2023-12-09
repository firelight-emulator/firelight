//
// Created by alexs on 11/7/2023.
//

#ifndef FIRELIGHT_WIDGET_PAINTER_HPP
#define FIRELIGHT_WIDGET_PAINTER_HPP

#include "../graphics/driver.hpp"
#include "../math/bbox.hpp"
#include "style.hpp"
#include <string>
namespace FL::GUI {

class WidgetPainter {
public:
  explicit WidgetPainter(FL::Graphics::Driver *driver);
  void paintText(const std::string &text, FL::Math::Box box, Style style);
  void paintBackground(FL::Math::Box box, Style style);

private:
  FL::Graphics::Driver *gfxDriver;
};

} // namespace FL::GUI

#endif // FIRELIGHT_WIDGET_PAINTER_HPP

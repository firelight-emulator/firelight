//
// Created by alexs on 10/30/2023.
//

#ifndef FIRELIGHT_STYLE_HPP
#define FIRELIGHT_STYLE_HPP

#include "../graphics/color.hpp"
namespace FL::GUI {

enum Alignment { LEFT, CENTER, RIGHT, TOP, BOTTOM };

enum Background { NO_FILL, FILL_BACKGROUND };

struct Style {
  Background background = NO_FILL;
  FL::Graphics::Color backgroundColor{};

  Alignment horiAlignment = LEFT;
  Alignment vertAlignment = TOP;

  //  bool hasBackgroundColor = false;
  //  FL::Graphics::Color backgroundColor{1.0, 1.0, 1.0, 1.0};
  //  TextAlignment horiTextAlignment = NONE;
  //  TextAlignment vertTextAlignment = NONE;
  int innerPaddingPx = 0;
  FL::Graphics::Color textColor{};
};

} // namespace FL::GUI

#endif // FIRELIGHT_STYLE_HPP

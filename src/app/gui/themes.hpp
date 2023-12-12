//
// Created by alexs on 12/11/2023.
//

#ifndef FIRELIGHT_THEMES_HPP
#define FIRELIGHT_THEMES_HPP

#include "../../lib/gui/style.hpp"
namespace FL::GUI {

class Themes {
public:
  constexpr static const FL::GUI::Style buttonFocus{
      .background = FILL_BACKGROUND,
      .backgroundColor = FL::Graphics::Color{.R = 1, .G = 1, .B = 1},
      .horiAlignment = LEFT,
      .vertAlignment = CENTER,
      .textColor = FL::Graphics::Color{}};

  constexpr static const FL::GUI::Style buttonInactive{
      .background = FILL_BACKGROUND,
      .backgroundColor = FL::Graphics::Color{.B = 1},
      .horiAlignment = LEFT,
      .vertAlignment = CENTER,
      .textColor = FL::Graphics::Color{.R = 1, .G = 1, .B = 1}};
};

} // namespace FL::GUI

#endif // FIRELIGHT_THEMES_HPP

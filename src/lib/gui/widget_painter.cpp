//
// Created by alexs on 11/7/2023.
//

#include "widget_painter.hpp"
#include "../graphics/driver.hpp"

namespace FL::GUI {

WidgetPainter::WidgetPainter(FL::Graphics::Driver *driver)
    : gfxDriver(driver) {}

void WidgetPainter::paintText(const std::string &text, FL::Math::Box box,
                              Style style) {
  auto cursorX = box.xPx;
  auto cursorY = box.yPx;

  int textW;
  int textH;
  gfxDriver->calculateTextBounds(text, textW, textH);

  if (style.vertAlignment == CENTER) {
    cursorY += (box.heightPx - textH) / 2;
  }

  if (style.horiAlignment == CENTER) {
    cursorX += (box.widthPx - textW) / 2;
  } else if (style.horiAlignment == RIGHT) {
    cursorX += (box.widthPx - textW);
  }

  // calculate padding, alignment etc
  gfxDriver->drawText(text, cursorX, cursorY, style.textColor);
}

void WidgetPainter::paintBackground(FL::Math::Box box, Style style) {
  if (style.background == NO_FILL) {
    return;
  }

  gfxDriver->drawRectangle(box.xPx, box.yPx, box.widthPx, box.heightPx,
                           style.backgroundColor);
}

} // namespace FL::GUI

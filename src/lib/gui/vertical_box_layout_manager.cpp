//
// Created by alexs on 11/12/2023.
//

#include "vertical_box_layout_manager.hpp"
#include "widgets/container_widget.hpp"

namespace FL::GUI {
VerticalBoxLayoutManager::VerticalBoxLayoutManager(int widgetHeightPx,
                                                   int spacingPx)
    : widgetHeightPx(widgetHeightPx), spacingPx(spacingPx) {}

void VerticalBoxLayoutManager::layoutWidgets(ContainerWidget &container) {
  auto cursorY = 0;

  auto width = container.getLayout().widthPx;

  for (auto &child : container.getChildren()) {
    child->box.xPx = 0;
    child->box.yPx = cursorY;
    child->box.widthPx = width;
    child->box.heightPx = widgetHeightPx;

    cursorY += widgetHeightPx + spacingPx;
  }
}
} // namespace FL::GUI

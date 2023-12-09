//
// Created by alexs on 11/12/2023.
//

#ifndef FIRELIGHT_VERTICAL_BOX_LAYOUT_MANAGER_HPP
#define FIRELIGHT_VERTICAL_BOX_LAYOUT_MANAGER_HPP

#include "layout_manager.hpp"
namespace FL::GUI {

class VerticalBoxLayoutManager : public LayoutManager {
public:
  VerticalBoxLayoutManager(int widgetHeightPx, int spacingPx);
  void layoutWidgets(ContainerWidget &container) override;

private:
  int widgetHeightPx;
  int spacingPx;
};

} // namespace FL::GUI

#endif // FIRELIGHT_VERTICAL_BOX_LAYOUT_MANAGER_HPP

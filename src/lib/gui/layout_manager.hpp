//
// Created by alexs on 11/12/2023.
//

#ifndef FIRELIGHT_LAYOUT_MANAGER_HPP
#define FIRELIGHT_LAYOUT_MANAGER_HPP

namespace FL::GUI {

class ContainerWidget;

class LayoutManager {
public:
  virtual void layoutWidgets(ContainerWidget &container) = 0;
  virtual ~LayoutManager() = default;
};

} // namespace FL::GUI

#endif // FIRELIGHT_LAYOUT_MANAGER_HPP

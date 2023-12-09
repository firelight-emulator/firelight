//
// Created by alexs on 11/9/2023.
//

#ifndef FIRELIGHT_CONTAINER_WIDGET_HPP
#define FIRELIGHT_CONTAINER_WIDGET_HPP

#include "../layout_manager.hpp"
#include "widget.hpp"
#include <memory>
namespace FL::GUI {

class ContainerWidget : public Widget {
public:
  void paint(WidgetPainter *painter, FL::Math::Box box) override;
  void addChild(std::unique_ptr<Widget> widget);
  bool focusable() override;
  Widget *getFirstFocusable() override;
  bool handleEvent(Event &event) override;
  bool isContainer() const override;
  std::vector<std::unique_ptr<Widget>> &getChildren();
  void setLayoutManager(std::unique_ptr<LayoutManager> manager);
  virtual ~ContainerWidget() = default;
  bool gainFocus() override;
  void loseFocus() override;

private:
  std::unique_ptr<LayoutManager> layoutManager;

  std::vector<std::unique_ptr<Widget>> childWidgets;
};

} // namespace FL::GUI

#endif // FIRELIGHT_CONTAINER_WIDGET_HPP

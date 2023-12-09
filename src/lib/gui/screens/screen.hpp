//
// Created by alexs on 11/13/2023.
//

#ifndef FIRELIGHT_SCREEN_HPP
#define FIRELIGHT_SCREEN_HPP

#include "../widgets/container_widget.hpp"
#include "../widgets/widget.hpp"
#include <memory>
namespace FL::GUI {

class Screen {
private:
  std::unique_ptr<ContainerWidget> mainContainer;
  Widget *focusTarget = nullptr;

public:
  explicit Screen(std::unique_ptr<ContainerWidget> container);
  virtual void forceStop();
  virtual void enter();
  virtual void exit();
  virtual void update(float deltaTime);
  virtual void render(const std::shared_ptr<WidgetPainter> &painter);

  virtual void setFocusTarget(Widget *target) { focusTarget = target; }
  virtual void setWindowArea(int width, int height);

  virtual bool handleEvent(Event &event);

  virtual void addWidget(std::unique_ptr<Widget> widget);
  virtual ~Screen() = default;

protected:
  int windowWidth = 0;
  int windowHeight = 0;
};

} // namespace FL::GUI

#endif // FIRELIGHT_SCREEN_HPP

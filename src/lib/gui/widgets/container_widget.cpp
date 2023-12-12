//
// Created by alexs on 11/9/2023.
//

#include "container_widget.hpp"
#include "imgui.h"
#include <sstream>

namespace FL::GUI {

void ContainerWidget::paint(WidgetPainter *painter, FL::Math::Box box) {
  //  painter->paintRectangle(box);

  if (layoutManager) {
    layoutManager->layoutWidgets(*this);
  }

  //  if (ImGui::TreeNode("Container")) {
  //    ImGui::BulletText(std::to_string(id).c_str());
  //
  //    //    for (auto f : focusChain) {
  //    //      std::ostringstream address;
  //    //      address << (void const *)f;
  //    //      std::string name = address.str();
  //    //
  //    //      if (ImGui::TreeNode(name.c_str())) {
  //    //        ImGui::TreePop();
  //    //      }
  //    //    }
  //    ImGui::TreePop();
  //  }

  for (auto &child : childWidgets) {
    child->paint(painter, child->box);
  }
}

bool ContainerWidget::focusable() {
  for (auto &child : childWidgets) {
    if (child->focusable()) {
      return true;
    }
  }

  return false;
}

bool ContainerWidget::handleEvent(Event &event) {
  if (event.type == Event::NAV_DOWN) {
    for (int i = 0; i < childWidgets.size(); ++i) {
      if (childWidgets.at(i)->isFocused && (i + 1) < childWidgets.size()) {
        childWidgets.at(i + 1)->gainFocus();
        childWidgets.at(i)->loseFocus();
        return true;
      }
    }
  } else if (event.type == Event::NAV_UP) {
    for (int i = 0; i < childWidgets.size(); ++i) {
      if (childWidgets.at(i)->isFocused && i > 0) {
        childWidgets.at(i - 1)->gainFocus();
        childWidgets.at(i)->loseFocus();
        return true;
      }
    }
  }

  return Widget::handleEvent(event);
}

Widget *ContainerWidget::getFirstFocusable() {
  for (auto &child : childWidgets) {
    if (child->focusable()) {
      auto addr = child->getFirstFocusable();
      return addr;
    }
  }

  return nullptr;
}

void ContainerWidget::addChild(std::unique_ptr<Widget> widget) {
  widget->setParent(this);
  childWidgets.push_back(std::move(widget));
}

bool ContainerWidget::gainFocus() {
  for (auto &child : childWidgets) {
    if (child->gainFocus()) {
      return true;
    }
  }

  return false;
}

void ContainerWidget::loseFocus() {
  for (auto &child : childWidgets) {
    child->loseFocus();
  }
}

bool ContainerWidget::isContainer() const { return true; }

std::vector<std::unique_ptr<Widget>> &ContainerWidget::getChildren() {
  return childWidgets;
}

void ContainerWidget::setLayoutManager(std::unique_ptr<LayoutManager> manager) {
  layoutManager = std::move(manager);
}

} // namespace FL::GUI

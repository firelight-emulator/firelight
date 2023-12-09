//
// Created by alexs on 11/9/2023.
//

#ifndef FIRELIGHT_EVENT_HPP
#define FIRELIGHT_EVENT_HPP

namespace FL::GUI {

class Event {
public:
  enum Type { NAV_UP, NAV_DOWN, NAV_SELECT_PUSHED, NAV_BACK_PUSHED, TEST };

  Type type;
};

typedef bool (*EventHandler)(FL::GUI::Event &);

} // namespace FL::GUI

#endif // FIRELIGHT_EVENT_HPP

//
// Created by alexs on 11/14/2023.
//

#include "screen_thing.hpp"

namespace FL::GUI {
ScreenThing::ScreenThing(ScreenManager *manager, WidgetFactory *factory)
    : screenManager(manager), widgetFactory(factory) {}
} // namespace FL::GUI

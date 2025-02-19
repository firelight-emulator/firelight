#include "router.hpp"

namespace firelight::gui {
    void Router::navigateTo(const QString &route) { emit routeChanged(route); }
    void Router::doSomethingWith(const QObject *thing,
                                 const QVariantMap &paramMap) {
      emit didSomethingWith(thing, paramMap);
    }
} // namespace firelight::gui

#include "router.hpp"

namespace firelight::gui {
    void Router::navigateTo(const QString &route) {
        emit routeChanged(route);
    }
}

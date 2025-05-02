#include "router.hpp"
#include <spdlog/spdlog.h>
#include <QUrl>

namespace firelight::gui {
    void Router::navigateTo(const QString &route) {
      

      auto url = QUrl(route);
      spdlog::info(url.path().toStdString());
      spdlog::info(url.query().toStdString());
      spdlog::info("matches: " + url.matches(QUrl("/shop/mods/:id"), QUrl::None) ? "true" : "false");
      spdlog::info(url.toString().toStdString());

      if (m_routeHistory.size() > 0 && m_routeHistory.last() == route) {
        return;
      }

      m_routeHistory.append(route);

      // for (int i = 0; i < m_routeHistory.size() - 1; i++) {
      //   spdlog::info("Route: " + m_routeHistory[i].toStdString());
      // }

      emit historySizeChanged();
      emit routeChanged(route); 
    }

    void Router::goBack() { 
      if (m_routeHistory.size() <= 1) {
        return;
      }

      m_routeHistory.removeLast();
      emit historySizeChanged();
      emit routeChanged(m_routeHistory.last());
    }
    } // namespace firelight::gui

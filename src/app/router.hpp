#pragma once

#include <QObject>

namespace firelight::gui {
    class Router : public QObject {
        Q_OBJECT

    public:
        Q_INVOKABLE void navigateTo(const QString &route);

    signals:
        void routeChanged(const QString &route);
    };
}

#pragma once

#include <QObject>

namespace firelight::gui {
    class Router : public QObject {
        Q_OBJECT

    public:
        Q_INVOKABLE void navigateTo(const QString &route);
        Q_INVOKABLE void doSomethingWith(const QObject *thing,
                                         const QVariantMap &paramMap);

    signals:
        void routeChanged(const QString &route);
        void didSomethingWith(const QObject *thing, const QVariantMap &paramMap);
    };
}

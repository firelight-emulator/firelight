#pragma once

#include <QObject>

namespace firelight::gui {
    class Router : public QObject {
        Q_OBJECT
        Q_PROPERTY(QString currentRoute READ currentRoute NOTIFY routeChanged)
        Q_PROPERTY(int historySize READ historySize NOTIFY historySizeChanged)

    public:
        Q_INVOKABLE void navigateTo(const QString &route);
        Q_INVOKABLE void goBack();

        QString currentRoute() const { 
            if (m_routeHistory.size() > 0) {
                return m_routeHistory.last();
            }
        
            return "";
        
        }
        int historySize() const { return m_routeHistory.size(); }
        
    signals:
        void routeChanged(const QString &route);
        void historySizeChanged();

    private:
        QStringList m_routeHistory;
    };
}

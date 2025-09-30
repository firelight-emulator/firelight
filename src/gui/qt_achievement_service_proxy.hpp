#pragma once
#include "service_accessor.hpp"

#include <QObject>
#include <event_dispatcher.hpp>

namespace firelight::gui {

class QtAchievementServiceProxy : public QObject, public ServiceAccessor {
  Q_OBJECT
  Q_PROPERTY(bool loggedIn READ isLoggedIn NOTIFY loggedInChanged)
  Q_PROPERTY(bool inHardcoreSession READ inHardcoreSession NOTIFY
                 inHardcoreSessionChanged)
  Q_PROPERTY(unsigned numCurrentSessionHardcoreUnlocks READ
                 numCurrentSessionHardcoreUnlocks NOTIFY
                     numCurrentSessionHardcoreUnlocksChanged)
public:
  explicit QtAchievementServiceProxy(QObject *parent = nullptr);

  bool isLoggedIn() const;
  bool inHardcoreSession() const;
  unsigned numCurrentSessionHardcoreUnlocks() const;
signals:
  void loggedInChanged();
  void inHardcoreSessionChanged();
  void numCurrentSessionHardcoreUnlocksChanged();

private:
  ScopedConnection m_sessionStartedConnection;
  ScopedConnection m_sessionEndedConnection;
};

} // namespace firelight::gui
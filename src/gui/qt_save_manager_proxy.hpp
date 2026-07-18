#pragma once

#include <firelight/saves/isave_manager.hpp>

#include <QObject>
#include <QString>

namespace firelight::gui {

// Thin QML adapter over the (Qt-notification-free) saves::ISaveManager. Its only
// job is to expose the save directory as a bindable Q_PROPERTY for the settings
// UI; the domain interface itself carries no QObject/signal concerns
class QtSaveManagerProxy : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString saveDirectory READ getSaveDirectory WRITE setSaveDirectory
                 NOTIFY saveDirectoryChanged)

public:
  explicit QtSaveManagerProxy(saves::ISaveManager &saveManager,
                              QObject *parent = nullptr);

  [[nodiscard]] QString getSaveDirectory() const;
  void setSaveDirectory(const QString &saveDirectory);

signals:
  void saveDirectoryChanged(const QString &saveDirectory);

private:
  saves::ISaveManager &m_saveManager;
};

} // namespace firelight::gui

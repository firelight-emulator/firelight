#include "qt_save_manager_proxy.hpp"

#include <QSettings>

namespace firelight::gui {

QtSaveManagerProxy::QtSaveManagerProxy(saves::ISaveManager &saveManager,
                                       QObject *parent)
    : QObject(parent), m_saveManager(saveManager) {}

QString QtSaveManagerProxy::getSaveDirectory() const {
  return QString::fromStdString(m_saveManager.getSaveDirectory());
}

void QtSaveManagerProxy::setSaveDirectory(const QString &saveDirectory) {
  const auto previous = m_saveManager.getSaveDirectory();
  m_saveManager.setSaveDirectory(saveDirectory.toStdString());
  const auto updated = m_saveManager.getSaveDirectory();
  if (updated != previous) {
    // Persist the setting here in the app layer; the Qt-free SaveManager no
    // longer owns QSettings. Startup reads this key back (see main.cpp).
    QSettings settings;
    settings.setValue("Saves/SaveDirectory",
                      QString::fromStdString(updated));
    emit saveDirectoryChanged(QString::fromStdString(updated));
  }
}

} // namespace firelight::gui

#include "qt_save_manager_proxy.hpp"

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
    emit saveDirectoryChanged(QString::fromStdString(updated));
  }
}

} // namespace firelight::gui

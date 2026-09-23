#pragma once

#include <firelight/saves/isave_manager.hpp>

#include <QObject>
#include <QString>
#include <QVariantList>

namespace firelight::gui {

class QtSaveManagerProxy : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString saveDirectory READ getSaveDirectory WRITE setSaveDirectory NOTIFY saveDirectoryChanged)

public:
  explicit QtSaveManagerProxy(saves::ISaveManager &saveManager, QObject *parent = nullptr);

  [[nodiscard]] QString getSaveDirectory() const;
  void setSaveDirectory(const QString &saveDirectory);

  [[nodiscard]] Q_INVOKABLE QVariantList getSaveFiles(const QString &contentHash) const;

signals:
  void saveDirectoryChanged(const QString &saveDirectory);

private:
  saves::ISaveManager &m_saveManager;
};

} // namespace firelight::gui

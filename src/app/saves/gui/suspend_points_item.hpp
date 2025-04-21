#pragma once
#include <manager_accessor.hpp>

namespace firelight::saves {
class SuspendPointsItem : public QObject, public ManagerAccessor {
  Q_OBJECT
  Q_PROPERTY(QString contentHash READ getContentHash WRITE setContentHash NOTIFY
                 contentHashChanged)
  Q_PROPERTY(int saveSlotNumber READ getSaveSlotNumber WRITE setSaveSlotNumber
                 NOTIFY saveSlotNumberChanged)
  Q_PROPERTY(QAbstractListModel *suspendPoints READ getSuspendPoints NOTIFY
                 suspendPointsChanged)

public:
  explicit SuspendPointsItem(QObject *parent = nullptr) : QObject(parent) {
    m_suspendPointsModel = new SuspendPointListModel(*getGameImageProvider());
  }

  [[nodiscard]] QString getContentHash() const;
  void setContentHash(const QString &contentHash);

  int getSaveSlotNumber() const;
  void setSaveSlotNumber(int saveSlotNumber);

  [[nodiscard]] SuspendPointListModel *getSuspendPoints() const;

signals:
  void contentHashChanged();
  void saveSlotNumberChanged();
  void suspendPointsChanged();

private:
  QString m_contentHash;
  int m_saveSlotNumber = -1;

  SuspendPointListModel *m_suspendPointsModel;
};

} // namespace firelight::saves
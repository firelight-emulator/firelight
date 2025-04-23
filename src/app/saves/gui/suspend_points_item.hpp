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

    connect(getSaveManager(), &SaveManager::suspendPointUpdated, this,
            [this](const QString &contentHash, int saveSlotNumber, int index) {
              if (contentHash != m_contentHash ||
                  saveSlotNumber != m_saveSlotNumber) {
                return;
              }

              const auto suspendPoint = getSaveManager()->readSuspendPoint(
                  contentHash, saveSlotNumber, index);
              if (suspendPoint.has_value()) {
                m_suspendPointsModel->updateData(index, *suspendPoint);
              }
            });

    connect(getSaveManager(), &SaveManager::suspendPointDeleted, this,
            [this](const QString &contentHash, int saveSlotNumber, int index) {
              if (contentHash != m_contentHash ||
                  saveSlotNumber != m_saveSlotNumber) {
                return;
              }

              m_suspendPointsModel->deleteData(index);
            });
  }

  [[nodiscard]] QString getContentHash() const;
  void setContentHash(const QString &contentHash);

  int getSaveSlotNumber() const;
  void setSaveSlotNumber(int saveSlotNumber);

  [[nodiscard]] SuspendPointListModel *getSuspendPoints() const;

  Q_INVOKABLE void deleteSuspendPoint(const int index) {
    getSaveManager()->deleteSuspendPoint(m_contentHash, m_saveSlotNumber,
                                         index);
  }

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
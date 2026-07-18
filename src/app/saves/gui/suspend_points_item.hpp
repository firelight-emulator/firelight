#pragma once
#include <service_accessor.hpp>
#include <firelight/event_dispatcher.hpp>
#include <firelight/saves/isave_manager.hpp>
#include <firelight/saves/save_events.hpp>
#include "suspend_point_list_model.hpp"

namespace firelight::saves {
class SuspendPointsItem : public QObject, public ServiceAccessor {
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

    m_suspendPointUpdatedConnection =
        EventDispatcher::instance().subscribe<SuspendPointUpdatedEvent>(
            [this](const SuspendPointUpdatedEvent &event) {
              if (QString::fromStdString(event.contentHash) != m_contentHash ||
                  event.saveSlotNumber != m_saveSlotNumber) {
                return;
              }

              const auto suspendPoint = getSaveManager()->readSuspendPoint(
                  m_contentHash.toStdString(), event.saveSlotNumber,
                  event.index);
              if (suspendPoint.has_value()) {
                m_suspendPointsModel->updateData(event.index, *suspendPoint);
              }
            });

    m_suspendPointDeletedConnection =
        EventDispatcher::instance().subscribe<SuspendPointDeletedEvent>(
            [this](const SuspendPointDeletedEvent &event) {
              if (QString::fromStdString(event.contentHash) != m_contentHash ||
                  event.saveSlotNumber != m_saveSlotNumber) {
                return;
              }

              m_suspendPointsModel->deleteData(event.index);
            });
  }

  [[nodiscard]] QString getContentHash() const;
  void setContentHash(const QString &contentHash);

  int getSaveSlotNumber() const;
  void setSaveSlotNumber(int saveSlotNumber);

  [[nodiscard]] SuspendPointListModel *getSuspendPoints() const;

  Q_INVOKABLE void deleteSuspendPoint(const int index) {
    getSaveManager()->deleteSuspendPoint(m_contentHash.toStdString(),
                                         m_saveSlotNumber, index);
  }

signals:
  void contentHashChanged();
  void saveSlotNumberChanged();
  void suspendPointsChanged();

private:
  QString m_contentHash;
  int m_saveSlotNumber = -1;

  SuspendPointListModel *m_suspendPointsModel;

  ScopedConnection m_suspendPointUpdatedConnection;
  ScopedConnection m_suspendPointDeletedConnection;
};

} // namespace firelight::saves

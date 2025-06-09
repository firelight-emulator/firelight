#pragma once
#include "savefile_list_model.hpp"
#include <manager_accessor.hpp>

namespace firelight::saves {
class SaveFilesItem : public QObject, public ManagerAccessor {
  Q_OBJECT
  Q_PROPERTY(int entryId READ getEntryId WRITE setEntryId NOTIFY entryIdChanged)
  Q_PROPERTY(int activeSaveSlotNumber READ getActiveSaveSlotNumber WRITE
                 setActiveSaveSlotNumber NOTIFY activeSaveSlotNumberChanged)
  Q_PROPERTY(
      QAbstractListModel *saveFiles READ getSaveFiles NOTIFY saveFilesChanged)

public:
  explicit SaveFilesItem(QObject *parent = nullptr) : QObject(parent) {
    m_saveFilesModel = new SavefileListModel();
  }

  [[nodiscard]] int getEntryId() const;
  void setEntryId(int entryId);

  [[nodiscard]] SavefileListModel *getSaveFiles() const;

  int getActiveSaveSlotNumber() const;
  void setActiveSaveSlotNumber(int activeSaveSlotNumber);

signals:
  void entryIdChanged();
  void saveFilesChanged();
  void activeSaveSlotNumberChanged();

private:
  int m_entryId = -1;
  int m_activeSaveSlotNumber = -1;

  SavefileListModel *m_saveFilesModel;
};

} // namespace firelight::saves
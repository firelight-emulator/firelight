#include "save_files_item.hpp"

namespace firelight::saves {

int SaveFilesItem::getEntryId() const { return m_entryId; }

void SaveFilesItem::setEntryId(const int entryId) {
  if (m_entryId == entryId) {
    return;
  }

  m_entryId = entryId;
  emit entryIdChanged();

  auto entry = getUserLibrary()->getEntry(m_entryId);
  if (!entry.has_value()) {
    spdlog::warn("SaveFilesItem: Entry with ID {} not found", m_entryId);
    return;
  }

  setActiveSaveSlotNumber(entry->activeSaveSlot);

  m_saveFilesModel->reset(
      getSaveManager()->getSaveFileInfoList(entry->contentHash));
}

SavefileListModel *SaveFilesItem::getSaveFiles() const {
  return m_saveFilesModel;
}

int SaveFilesItem::getActiveSaveSlotNumber() const {
  return m_activeSaveSlotNumber;
}

void SaveFilesItem::setActiveSaveSlotNumber(const int activeSaveSlotNumber) {
  if (m_activeSaveSlotNumber == activeSaveSlotNumber) {
    return;
  }

  m_activeSaveSlotNumber = activeSaveSlotNumber;
  emit activeSaveSlotNumberChanged();

  auto entry = getUserLibrary()->getEntry(m_entryId);
  if (!entry.has_value()) {
    spdlog::warn("SaveFilesItem: Entry with ID {} not found", m_entryId);
    return;
  }

  entry->activeSaveSlot = activeSaveSlotNumber;
  if (!getUserLibrary()->update(*entry)) {
    spdlog::error("SaveFilesItem: Failed to update entry with ID {}",
                  m_entryId);
  }
}

} // namespace firelight::saves
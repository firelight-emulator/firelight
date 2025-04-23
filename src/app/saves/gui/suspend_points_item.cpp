#include "suspend_points_item.hpp"

namespace firelight::saves {
QString SuspendPointsItem::getContentHash() const { return m_contentHash; }
void SuspendPointsItem::setContentHash(const QString &contentHash) {
  if (m_contentHash == contentHash) {
    return;
  }

  m_contentHash = contentHash;

  for (auto i = 0; i < 16; ++i) {
    auto point =
        getSaveManager()->readSuspendPoint(m_contentHash, m_saveSlotNumber, i);
    if (point.has_value()) {
      m_suspendPointsModel->updateData(i, point.value());
    }
  }

  emit suspendPointsChanged();
  emit contentHashChanged();
}

int SuspendPointsItem::getSaveSlotNumber() const { return m_saveSlotNumber; }
void SuspendPointsItem::setSaveSlotNumber(const int saveSlotNumber) {
  if (m_saveSlotNumber == saveSlotNumber) {
    return;
  }

  m_saveSlotNumber = saveSlotNumber;

  for (auto i = 0; i < 16; ++i) {
    auto point =
        getSaveManager()->readSuspendPoint(m_contentHash, m_saveSlotNumber, i);
    if (point.has_value()) {
      m_suspendPointsModel->updateData(i, point.value());
    }
  }

  emit suspendPointsChanged();
  emit saveSlotNumberChanged();
}

SuspendPointListModel *SuspendPointsItem::getSuspendPoints() const {
  return m_suspendPointsModel;
}
} // namespace firelight::saves
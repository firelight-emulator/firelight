#include "suspend_points_item.hpp"

namespace firelight::saves {
QString SuspendPointsItem::getContentHash() const { return m_contentHash; }
void SuspendPointsItem::setContentHash(const QString &contentHash) {
  if (m_contentHash == contentHash) {
    return;
  }

  m_contentHash = contentHash;

  auto points =
      getSaveManager()->getSuspendPoints(m_contentHash, m_saveSlotNumber);
  for (int i = 0; i < points.size(); ++i) {
    m_suspendPointsModel->updateData(i, points[i]);
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

  auto points =
      getSaveManager()->getSuspendPoints(m_contentHash, m_saveSlotNumber);
  for (int i = 0; i < points.size(); ++i) {
    m_suspendPointsModel->updateData(i, points[i]);
  }

  emit suspendPointsChanged();
  emit saveSlotNumberChanged();
}

SuspendPointListModel *SuspendPointsItem::getSuspendPoints() const {
  return m_suspendPointsModel;
}
} // namespace firelight::saves
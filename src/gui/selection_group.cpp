// TODO: NEEDS REVIEW
#include "selection_group.hpp"

namespace firelight::gui {
SelectionGroup::SelectionGroup(QObject *parent) : QObject(parent) {}

QVariantMap SelectionGroup::getSelected() const {
  if (!m_isCacheDirty) {
    return m_selectedCache;
  }

  m_selectedCache.clear();

  for (auto it = m_selectedIndices.cbegin(); it != m_selectedIndices.cend(); ++it) {
    m_selectedCache.insert(QString::number(it.key()), it.value());
  }

  m_isCacheDirty = false;

  return m_selectedCache;
}

void SelectionGroup::markChanged() {
  m_isCacheDirty = true;
  emit selectedChanged();
}

bool SelectionGroup::isActive() const { return m_active; }

void SelectionGroup::setActive(const bool active) {
  if (m_active != active) {
    m_active = active;
    emit activeChanged();

    if (!active) {
      clearSelection();
    }
  }
}

bool SelectionGroup::areMultipleSelected() const { return m_selectedIndices.size() > 1; }

int SelectionGroup::getRangeStart() const { return m_rangeStart; }

int SelectionGroup::getRangeEnd() const { return m_rangeEnd; }

bool SelectionGroup::isEditingRange() const { return m_editingRange; }

void SelectionGroup::setEditingRange(const bool editing) {
  if (m_editingRange != editing) {
    m_editingRange = editing;
    markChanged();
  }
}

void SelectionGroup::select(const int index, const int modifiersMask) {
  if (modifiersMask & Qt::ShiftModifier) {
    paintTo(index);
    return;
  }

  toggle(index);
}

void SelectionGroup::selectAll(const int count) {
  if (count <= 0) {
    clearSelection();
    return;
  }

  m_selectedIndices.clear();

  for (auto i = 0; i < count; ++i) {
    m_selectedIndices.insert(i, true);
  }

  m_baseline = m_selectedIndices;
  m_anchorIndex = -1;
  m_rangeStart = -1;
  m_rangeEnd = -1;
  m_editingRange = false;

  markChanged();
}

void SelectionGroup::clearSelection() {
  if (m_selectedIndices.isEmpty() && m_anchorIndex < 0 && !m_editingRange) {
    return;
  }

  m_selectedIndices.clear();
  m_baseline.clear();
  m_anchorIndex = -1;
  m_rangeStart = -1;
  m_rangeEnd = -1;
  m_editingRange = false;

  markChanged();
}

bool SelectionGroup::isSelected(const int index) const { return m_selectedIndices.contains(index); }

void SelectionGroup::toggle(const int index) {
  const auto turningOn = !m_selectedIndices.contains(index);

  if (turningOn) {
    m_selectedIndices.insert(index, true);
  } else {
    m_selectedIndices.remove(index);
  }

  m_rangeStart = -1;
  m_rangeEnd = -1;
  m_editingRange = false;

  m_anchorIndex = index;
  m_anchorState = turningOn;
  m_baseline = m_selectedIndices;

  markChanged();
}

void SelectionGroup::paintTo(const int index) {
  if (m_anchorIndex < 0) {
    toggle(index);
    return;
  }

  m_selectedIndices = m_baseline;

  const auto lo = std::min(m_anchorIndex, index);
  const auto hi = std::max(m_anchorIndex, index);

  for (auto i = lo; i <= hi; ++i) {
    if (m_anchorState) {
      m_selectedIndices.insert(i, true);
    } else {
      m_selectedIndices.remove(i);
    }
  }

  m_rangeStart = m_anchorIndex;
  m_rangeEnd = index;
  m_editingRange = true;

  markChanged();
}

} // namespace firelight::gui
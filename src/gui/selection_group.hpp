// TODO: NEEDS REVIEW
#pragma once
#include <QVariantMap>

namespace firelight::gui {

class SelectionGroup : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)
  Q_PROPERTY(QVariantMap selected READ getSelected NOTIFY selectedChanged)
  Q_PROPERTY(bool multipleSelected READ areMultipleSelected NOTIFY selectedChanged)
  Q_PROPERTY(int rangeStart READ getRangeStart NOTIFY selectedChanged)
  Q_PROPERTY(int rangeEnd READ getRangeEnd NOTIFY selectedChanged)
  Q_PROPERTY(bool editingRange READ isEditingRange WRITE setEditingRange NOTIFY selectedChanged)

public:
  explicit SelectionGroup(QObject *parent = nullptr);

  [[nodiscard]] bool isActive() const;
  void setActive(bool active);

  [[nodiscard]] QVariantMap getSelected() const;

  [[nodiscard]] bool areMultipleSelected() const;

  [[nodiscard]] int getRangeStart() const;
  [[nodiscard]] int getRangeEnd() const;

  [[nodiscard]] bool isEditingRange() const;
  void setEditingRange(bool editing);

  Q_INVOKABLE void select(int index, int modifiersMask);

  // TODO
  /** Selects indices 0 to count - 1 and nothing else */
  Q_INVOKABLE void selectAll(int count);

  Q_INVOKABLE void clearSelection();
  Q_INVOKABLE bool isSelected(int index) const;

signals:
  void activeChanged();
  void selectedChanged();

private:
  void markChanged();
  void paintTo(int index);
  void toggle(int index);

  bool m_editingRange = false;

  int m_anchorIndex = -1; // Used for shift+click
  bool m_anchorState = false;
  QMap<int, bool> m_baseline;

  int m_rangeStart = -1;
  int m_rangeEnd = -1;

  QMap<int, bool> m_selectedIndices;
  bool m_active = false;

  mutable QVariantMap m_selectedCache;
  mutable bool m_isCacheDirty = true;
};

} // namespace firelight::gui
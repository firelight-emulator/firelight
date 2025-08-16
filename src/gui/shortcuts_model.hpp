#pragma once
#include "input2/input_service.hpp"

#include <QAbstractListModel>

namespace firelight::gui {

class ShortcutsModel : public QAbstractListModel {
  Q_OBJECT

public:
  explicit ShortcutsModel(
      bool isKeyboard,
      const std::shared_ptr<input::ShortcutMapping> &shortcutMapping);
  int rowCount(const QModelIndex &parent) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  bool setData(const QModelIndex &index, const QVariant &value,
               int role) override;

  Q_INVOKABLE void setMapping(int shortcut, QList<int> modifiers, int input);

private:
  enum Roles {
    Shortcut = Qt::UserRole + 1,
    Name,
    HasMapping,
    HasConflict,
    Modifiers,
    ModifierNames,
    Input,
    InputName
  };

  struct Item {
    input::Shortcut shortcut;
    QString shortcutName;
    bool hasMapping = false;
    bool hasConflict = false;
    QList<int> modifiers;
    QStringList modifierNames;
    int input;
    QString inputName;
  };

  input::InputService *m_inputService;
  std::shared_ptr<input::ShortcutMapping> m_shortcutMapping;
  int m_profileId = -1;
  bool m_isKeyboard;

  QList<Item> m_items;
};

} // namespace firelight::gui
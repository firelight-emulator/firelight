#pragma once
#include <firelight/input/input_source.hpp>
#include <firelight/input/shortcut_mapping.hpp>
#include "service_accessor.hpp"

#include <QAbstractListModel>
#include <memory>

namespace firelight::gui {

// Lists the global shortcut catalog (from ShortcutRegistry) alongside the
// bindings a single profile has for each action. One row per action.
class ShortcutsModel : public QAbstractListModel, public ServiceAccessor {
  Q_OBJECT

public:
  explicit ShortcutsModel(
      bool isKeyboard,
      std::shared_ptr<input::ShortcutMapping> shortcutMapping);

  int rowCount(const QModelIndex &parent) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  // Appends a binding to a shortcut. `modifiers`/`input` are GamepadInput codes
  // for a controller profile, or Qt::Key codes for a keyboard profile.
  Q_INVOKABLE void addBinding(const QString &shortcutId, QList<int> modifiers,
                              int input);
  Q_INVOKABLE void clearBindings(const QString &shortcutId);

private:
  enum Roles {
    ShortcutIdRole = Qt::UserRole + 1,
    NameRole,
    CategoryRole,
    ActivationRole,
    HasBindingRole,
    BindingsLabelRole,
  };

  struct Item {
    QString id;
    QString name;
    QString category;
    int activation = 0;
    bool hasBinding = false;
    QString bindingsLabel;
  };

  void rebuild();
  void refreshRow(const QString &id);
  int indexOfId(const QString &id) const;
  QString labelForBindings(const std::vector<input::InputSource> &sources) const;

  std::shared_ptr<input::ShortcutMapping> m_shortcutMapping;
  bool m_isKeyboard;
  QList<Item> m_items;
};

} // namespace firelight::gui

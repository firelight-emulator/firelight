#include "shortcuts_model.hpp"

#include <firelight/input/keyboard_input_handler.hpp>
#include <firelight/input/shortcut_registry.hpp>
#include <firelight/input/gamepad_input.hpp>

#include <algorithm>
#include <utility>

namespace firelight::gui {

ShortcutsModel::ShortcutsModel(
    const bool isKeyboard,
    std::shared_ptr<input::ShortcutMapping> shortcutMapping)
    : QAbstractListModel(nullptr),
      m_shortcutMapping(std::move(shortcutMapping)), m_isKeyboard(isKeyboard) {
  rebuild();
}

QString ShortcutsModel::labelForBindings(
    const std::vector<input::InputSource> &sources) const {
  const auto label = [this](const int code) {
    return m_isKeyboard
               ? input::KeyboardInputHandler::getKeyLabel(
                     static_cast<Qt::Key>(code))
               : QString::fromStdString(firelight::input::displayName(
                     static_cast<input::GamepadInput>(code)));
  };

  QStringList combos;
  for (const auto &source : sources) {
    QStringList parts;
    for (const auto mod : source.modifiers) {
      parts << label(mod);
    }
    parts << label(source.code);
    combos << parts.join(" + ");
  }
  return combos.join(",  ");
}

void ShortcutsModel::rebuild() {
  beginResetModel();
  m_items.clear();

  auto actions = input::ShortcutRegistry::instance().listActions();
  std::sort(actions.begin(), actions.end(),
            [](const input::ShortcutAction &a, const input::ShortcutAction &b) {
              if (a.category != b.category) {
                return a.category < b.category;
              }
              return a.displayName < b.displayName;
            });

  for (const auto &action : actions) {
    Item item;
    item.id = QString::fromStdString(action.id);
    item.name = QString::fromStdString(action.displayName);
    item.category = QString::fromStdString(action.category);
    item.activation = static_cast<int>(action.activation);
    if (m_shortcutMapping) {
      const auto &bindings = m_shortcutMapping->getBindings(action.id);
      item.hasBinding = !bindings.empty();
      item.bindingsLabel =
          item.hasBinding ? labelForBindings(bindings) : QStringLiteral("Not bound");
    } else {
      item.bindingsLabel = QStringLiteral("Not bound");
    }
    m_items.append(item);
  }
  endResetModel();
}

void ShortcutsModel::refreshRow(const QString &id) {
  const int row = indexOfId(id);
  if (row < 0 || !m_shortcutMapping) {
    return;
  }
  auto &item = m_items[row];
  const auto &bindings = m_shortcutMapping->getBindings(id.toStdString());
  item.hasBinding = !bindings.empty();
  item.bindingsLabel =
      item.hasBinding ? labelForBindings(bindings) : QStringLiteral("Not bound");
  emit dataChanged(index(row), index(row));
}

int ShortcutsModel::indexOfId(const QString &id) const {
  for (int i = 0; i < m_items.size(); ++i) {
    if (m_items[i].id == id) {
      return i;
    }
  }
  return -1;
}

int ShortcutsModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return 0;
  }
  return static_cast<int>(m_items.size());
}

QVariant ShortcutsModel::data(const QModelIndex &index, const int role) const {
  if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size()) {
    return {};
  }
  const auto &item = m_items.at(index.row());
  switch (role) {
  case ShortcutIdRole:
    return item.id;
  case NameRole:
    return item.name;
  case CategoryRole:
    return item.category;
  case ActivationRole:
    return item.activation;
  case HasBindingRole:
    return item.hasBinding;
  case BindingsLabelRole:
    return item.bindingsLabel;
  default:
    return {};
  }
}

QHash<int, QByteArray> ShortcutsModel::roleNames() const {
  return {
      {ShortcutIdRole, "shortcutId"}, {NameRole, "name"},
      {CategoryRole, "category"},     {ActivationRole, "activation"},
      {HasBindingRole, "hasBinding"}, {BindingsLabelRole, "bindingsLabel"},
  };
}

void ShortcutsModel::addBinding(const QString &shortcutId, QList<int> modifiers,
                                const int input) {
  if (!m_shortcutMapping) {
    return;
  }
  input::InputSource source;
  source.type =
      m_isKeyboard ? input::SourceType::Key : input::SourceType::Button;
  source.code = input;
  for (const auto mod : modifiers) {
    source.modifiers.push_back(mod);
  }
  m_shortcutMapping->addBinding(shortcutId.toStdString(), source);
  m_shortcutMapping->sync();
  refreshRow(shortcutId);
}

void ShortcutsModel::clearBindings(const QString &shortcutId) {
  if (!m_shortcutMapping) {
    return;
  }
  m_shortcutMapping->setBindings(shortcutId.toStdString(), {});
  m_shortcutMapping->sync();
  refreshRow(shortcutId);
}

} // namespace firelight::gui

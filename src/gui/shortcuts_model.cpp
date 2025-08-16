#include "shortcuts_model.hpp"

#include <input/keyboard_input_handler.hpp>
#include <platform_metadata.hpp>

namespace firelight::gui {
ShortcutsModel::ShortcutsModel(
    const bool isKeyboard,
    const std::shared_ptr<input::ShortcutMapping> &shortcutMapping)
    : QAbstractListModel(nullptr), m_shortcutMapping(shortcutMapping),
      m_isKeyboard(isKeyboard) {
  m_inputService = input::InputService::instance();

  beginResetModel();
  for (const auto &shortcut : input::InputService::listShortcuts()) {
    Item item;
    item.shortcut = shortcut.first;
    item.shortcutName = QString::fromStdString(shortcut.second);
    item.hasMapping = m_shortcutMapping->getMappings().contains(shortcut.first);
    item.hasConflict = false;

    item.modifiers.clear();
    item.modifierNames.clear();

    if (item.hasMapping) {
      for (const auto &input :
           m_shortcutMapping->getMappings().at(shortcut.first).modifiers) {
        item.modifiers.append(input);
        if (m_isKeyboard) {
          item.modifierNames.append(input::KeyboardInputHandler::getKeyLabel(
              static_cast<Qt::Key>(input)));
        } else {
          item.modifierNames.append(
              QString::fromStdString(PlatformMetadata::getInputName(
                  static_cast<input::GamepadInput>(input))));
        }
      }

      item.input = m_shortcutMapping->getMappings()
                       .at(shortcut.first)
                       .input; // Assuming input is GamepadInput
      if (m_isKeyboard) {
        item.inputName = input::KeyboardInputHandler::getKeyLabel(
            static_cast<Qt::Key>(item.input));
      } else {
        item.inputName = QString::fromStdString(PlatformMetadata::getInputName(
            static_cast<input::GamepadInput>(item.input)));
      }
    } else {
      item.input = input::GamepadInput::None; // Default value if not mapped
      item.inputName = "Not mapped";
    }

    m_items.emplace_back(item);
  }

  endResetModel();
}
int ShortcutsModel::rowCount(const QModelIndex &parent) const {
  return m_items.size();
}
QVariant ShortcutsModel::data(const QModelIndex &index, const int role) const {
  if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size()) {
    return {};
  }

  const auto &item = m_items.at(index.row());
  switch (role) {
  case Shortcut:
    return item.shortcut;
  case Name:
    return item.shortcutName;
  case HasMapping:
    return item.hasMapping;
  case HasConflict:
    return item.hasConflict;
  case Modifiers:
    return QVariant::fromValue(item.modifiers);
  case ModifierNames:
    return item.modifierNames;
  case Input:
    return item.input;
  case InputName:
    return item.inputName;
  default:
    return {};
  }
}

QHash<int, QByteArray> ShortcutsModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[Shortcut] = "shortcut";
  roles[Name] = "name";
  roles[HasMapping] = "hasMapping";
  roles[HasConflict] = "hasConflict";
  roles[Modifiers] = "modifiers";
  roles[ModifierNames] = "modifierNames";
  roles[Input] = "input";
  roles[InputName] = "inputName";
  return roles;
}

Qt::ItemFlags ShortcutsModel::flags(const QModelIndex &index) const {
  return Qt::ItemIsEditable | QAbstractListModel::flags(index);
}

bool ShortcutsModel::setData(const QModelIndex &index, const QVariant &value,
                             const int role) {
  if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size()) {
    return false;
  }

  auto &item = m_items[index.row()];
  switch (role) {
  case HasMapping:
    item.hasMapping = false;
    item.input = input::GamepadInput::None; // Default value if not mapped
    item.inputName = "Not mapped";
    item.hasConflict = false;

    item.modifiers.clear();
    item.modifierNames.clear();

    m_shortcutMapping->clearMapping(item.shortcut);
    m_shortcutMapping->sync();
    break;
  default:
    return false;
  }

  emit dataChanged(index, index);

  return true;
}

void ShortcutsModel::setMapping(int shortcut, QList<int> modifiers, int input) {
  for (auto i = 0; i < m_items.size(); ++i) {
    auto &item = m_items[i];

    if (item.shortcut != shortcut) {
      continue;
    }

    item.modifiers.clear();
    item.modifierNames.clear();

    for (const auto &modifier : modifiers) {
      item.modifiers.append(modifier);
      if (m_isKeyboard) {
        item.modifierNames.append(input::KeyboardInputHandler::getKeyLabel(
            static_cast<Qt::Key>(modifier)));
      } else {
        item.modifierNames.append(
            QString::fromStdString(PlatformMetadata::getInputName(
                static_cast<input::GamepadInput>(modifier))));
      }
    }

    item.input = static_cast<input::GamepadInput>(input);
    if (m_isKeyboard) {
      item.inputName = input::KeyboardInputHandler::getKeyLabel(
          static_cast<Qt::Key>(item.input));
    } else {
      item.inputName = QString::fromStdString(PlatformMetadata::getInputName(
          static_cast<input::GamepadInput>(item.input)));
    }

    // TODO: Check for conflicts
    item.hasConflict = false;
    item.hasMapping = true;

    std::vector<int> modifierList;
    for (const auto &mod : item.modifiers) {
      modifierList.push_back(mod);
    }

    m_shortcutMapping->setMapping(item.shortcut, {modifierList, item.input});
    m_shortcutMapping->sync();

    emit dataChanged(index(i), index(i));
    return;
  }
}
} // namespace firelight::gui
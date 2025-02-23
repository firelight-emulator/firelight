#include "keyboard_mapping_item.hpp"

namespace firelight::input {
KeyboardMappingItem::KeyboardMappingItem(QObject *parent) {
  refreshMapping();
}

QVariantMap KeyboardMappingItem::getInputMappings() const {
  return m_inputMappings;
}

int KeyboardMappingItem::getProfileId() const {
  return m_profileId;
}

int KeyboardMappingItem::getPlatformId() const {
  return m_platformId;
}

void KeyboardMappingItem::setProfileId(const int profileId) {
  if (profileId == m_profileId) {
    return;
  }

  m_profileId = profileId;
  emit profileIdChanged();
  refreshMapping();
}

void KeyboardMappingItem::setPlatformId(const int platformId) {
  if (platformId == m_platformId) {
    return;
  }

  m_platformId = platformId;
  emit platformIdChanged();
  refreshMapping();
}

void KeyboardMappingItem::addMapping(int input, const int mappedInput) {
  m_inputMapping->addKeyboardMapping(
      static_cast<libretro::IRetroPad::Input>(input), mappedInput);
  m_inputMapping->sync();
  m_inputMappings.insert(QString::number(input), mappedInput);
  emit inputMappingsChanged();
}

void KeyboardMappingItem::removeAllMappings() {
  for (auto key: m_inputMappings.keys()) {
    m_inputMapping->removeMapping(static_cast<libretro::IRetroPad::Input>(key.toInt()));
    m_inputMappings.remove(key);
  }

  m_inputMapping->sync();
  emit inputMappingsChanged();
}

void KeyboardMappingItem::removeMapping(int input) {
  m_inputMapping->removeMapping(static_cast<libretro::IRetroPad::Input>(input));
  m_inputMapping->sync();
  m_inputMappings.remove(QString::number(input));
  emit inputMappingsChanged();
}

QString KeyboardMappingItem::getKeyLabel(int input) const {
  if (QChar::isPrint(input)) {
    return QChar(input);
  }

  switch (static_cast<Qt::Key>(input)) {
    case Qt::Key_Shift : return QString("Shift");
    case Qt::Key_Control : return QString("Control");
    case Qt::Key_Alt  : return QString("Alt");
    case Qt::Key_Backspace: return "Backspace";
    case Qt::Key_Tab: return "Tab";
    case Qt::Key_Delete: return "Delete";
    case Qt::Key_Return: return "Return";
    case Qt::Key_Escape: return "Escape";
    case Qt::Key_Space: return "Space";
    case Qt::Key_Enter: return "Enter";
    case Qt::Key_Left: return "Left";
    case Qt::Key_Right: return "Right";
    case Qt::Key_Up: return "Up";
    case Qt::Key_Down: return "Down";
    case Qt::Key_PageUp: return "PageUp";
    case Qt::Key_PageDown: return "PageDown";
    case Qt::Key_Home: return "Home";
    case Qt::Key_End: return "End";
    case Qt::Key_Insert: return "Insert";
    case Qt::Key_Underscore: return "Underscore";
    case Qt::Key_Backslash: return "Backslash";
    case Qt::Key_Slash: return "Slash";
    case Qt::Key_Semicolon: return "Semicolon";
    case Qt::Key_Comma: return "Comma";
    case Qt::Key_Period: return "Period";
    default: return "Unknown?";
  }
}

QString KeyboardMappingItem::getDefaultKeyLabel(int input) const {
  switch (static_cast<libretro::IRetroPad::Input>(input)) {
  case libretro::IRetroPad::LeftTrigger: return "E";
  case libretro::IRetroPad::RightTrigger: return "R";
  case libretro::IRetroPad::RightBumper: return "W";
  case libretro::IRetroPad::LeftBumper: return "Q";
  case libretro::IRetroPad::NorthFace: return "S";
  case libretro::IRetroPad::EastFace: return "X";
  case libretro::IRetroPad::WestFace: return "A";
  case libretro::IRetroPad::SouthFace: return "Z";
  case libretro::IRetroPad::LeftStickUp: return "Up";
  case libretro::IRetroPad::LeftStickDown: return "Down";
  case libretro::IRetroPad::LeftStickLeft: return "Left";
  case libretro::IRetroPad::LeftStickRight: return "Right";
  case libretro::IRetroPad::DpadUp: return "I";
  case libretro::IRetroPad::DpadDown: return "K";
  case libretro::IRetroPad::DpadLeft: return "J";
  case libretro::IRetroPad::DpadRight: return "L";
  case libretro::IRetroPad::Start: return "Return";
  case libretro::IRetroPad::Select: return "Shift";
  default: return "Unknown?";
  }
}

void KeyboardMappingItem::refreshMapping() {
  m_inputMappings.clear();
  if (m_profileId == 0 && m_platformId == 0) {
    return;
  }

  m_inputMapping = std::static_pointer_cast<KeyboardMapping>(getControllerRepository()->getInputMapping(m_profileId, m_platformId));
  for (const auto &[input, mappedInput]: m_inputMapping->getKeyboardMappings()) {
    m_inputMappings.insert(QString::number(input), mappedInput);
  }
  emit inputMappingsChanged();
}
} // firelight::input

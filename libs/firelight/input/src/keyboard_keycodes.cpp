#include <firelight/input/keyboard_keycodes.hpp>

#include <QtCore/qnamespace.h>
#include <map>

// TODO
// RETROK_* / RETROKMOD_* values, copied rather than pulled in: this library
// doesn't otherwise depend on the libretro headers, and these are frozen ABI —
// they can't change without breaking every core
namespace {
enum {
  RETROK_BACKSPACE = 8,
  RETROK_TAB = 9,
  RETROK_RETURN = 13,
  RETROK_PAUSE = 19,
  RETROK_ESCAPE = 27,
  RETROK_SPACE = 32,
  RETROK_COMMA = 44,
  RETROK_MINUS = 45,
  RETROK_PERIOD = 46,
  RETROK_SLASH = 47,
  RETROK_0 = 48,
  RETROK_SEMICOLON = 59,
  RETROK_EQUALS = 61,
  RETROK_LEFTBRACKET = 91,
  RETROK_BACKSLASH = 92,
  RETROK_RIGHTBRACKET = 93,
  RETROK_a = 97,
  RETROK_DELETE = 127,
  RETROK_UP = 273,
  RETROK_DOWN = 274,
  RETROK_RIGHT = 275,
  RETROK_LEFT = 276,
  RETROK_INSERT = 277,
  RETROK_HOME = 278,
  RETROK_END = 279,
  RETROK_PAGEUP = 280,
  RETROK_PAGEDOWN = 281,
  RETROK_F1 = 282,
  RETROK_NUMLOCK = 300,
  RETROK_CAPSLOCK = 301,
  RETROK_SCROLLOCK = 302,
  RETROK_RSHIFT = 303,
  RETROK_LSHIFT = 304,
  RETROK_RCTRL = 305,
  RETROK_LCTRL = 306,
  RETROK_RALT = 307,
  RETROK_LALT = 308,
  RETROK_LSUPER = 311,
};
} // namespace

namespace firelight::input {

unsigned retroKeyFor(const int qtKey) {
  // Letters and digits are contiguous in both, so they map arithmetically
  // RETROK follows ASCII, and its letters are lowercase
  if (qtKey >= Qt::Key_A && qtKey <= Qt::Key_Z) {
    return RETROK_a + (qtKey - Qt::Key_A);
  }
  if (qtKey >= Qt::Key_0 && qtKey <= Qt::Key_9) {
    return RETROK_0 + (qtKey - Qt::Key_0);
  }
  if (qtKey >= Qt::Key_F1 && qtKey <= Qt::Key_F15) {
    return RETROK_F1 + (qtKey - Qt::Key_F1);
  }

  static const std::map<int, unsigned> BY_KEY = {
      {Qt::Key_Backspace, RETROK_BACKSPACE},
      {Qt::Key_Tab, RETROK_TAB},
      {Qt::Key_Return, RETROK_RETURN},
      {Qt::Key_Enter, RETROK_RETURN},
      {Qt::Key_Pause, RETROK_PAUSE},
      {Qt::Key_Escape, RETROK_ESCAPE},
      {Qt::Key_Space, RETROK_SPACE},
      {Qt::Key_Comma, RETROK_COMMA},
      {Qt::Key_Minus, RETROK_MINUS},
      {Qt::Key_Period, RETROK_PERIOD},
      {Qt::Key_Slash, RETROK_SLASH},
      {Qt::Key_Semicolon, RETROK_SEMICOLON},
      {Qt::Key_Equal, RETROK_EQUALS},
      {Qt::Key_BracketLeft, RETROK_LEFTBRACKET},
      {Qt::Key_Backslash, RETROK_BACKSLASH},
      {Qt::Key_BracketRight, RETROK_RIGHTBRACKET},
      {Qt::Key_Delete, RETROK_DELETE},
      {Qt::Key_Up, RETROK_UP},
      {Qt::Key_Down, RETROK_DOWN},
      {Qt::Key_Right, RETROK_RIGHT},
      {Qt::Key_Left, RETROK_LEFT},
      {Qt::Key_Insert, RETROK_INSERT},
      {Qt::Key_Home, RETROK_HOME},
      {Qt::Key_End, RETROK_END},
      {Qt::Key_PageUp, RETROK_PAGEUP},
      {Qt::Key_PageDown, RETROK_PAGEDOWN},
      {Qt::Key_NumLock, RETROK_NUMLOCK},
      {Qt::Key_CapsLock, RETROK_CAPSLOCK},
      {Qt::Key_ScrollLock, RETROK_SCROLLOCK},
      {Qt::Key_Shift, RETROK_LSHIFT},
      {Qt::Key_Control, RETROK_LCTRL},
      {Qt::Key_Alt, RETROK_LALT},
      {Qt::Key_Meta, RETROK_LSUPER},
  };

  const auto it = BY_KEY.find(qtKey);
  return it == BY_KEY.end() ? RETROK_UNKNOWN : it->second;
}

uint16_t retroModifiersFor(const int qtModifiers) {
  uint16_t mods = 0;
  if (qtModifiers & Qt::ShiftModifier) {
    mods |= RETROKMOD_SHIFT;
  }
  if (qtModifiers & Qt::ControlModifier) {
    mods |= RETROKMOD_CTRL;
  }
  if (qtModifiers & Qt::AltModifier) {
    mods |= RETROKMOD_ALT;
  }
  if (qtModifiers & Qt::MetaModifier) {
    mods |= RETROKMOD_META;
  }
  return mods;
}

} // namespace firelight::input

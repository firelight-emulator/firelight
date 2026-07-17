#pragma once
#include <firelight/input/gamepad_type.hpp>
#include <firelight/input/shortcut_action.hpp>

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace firelight::input {

// A named set of shipped bindings, offered when a profile is created.
//
// A preset is a *starting point*, not a tier: seeding copies it into the profile
// once, and the profile owns a flat mapping from then on. So the engine never
// resolves through a preset, an unbound action just means "off", and "reset this
// row" is a write rather than an erase.
struct ShortcutPreset {
  std::string id;
  std::string label;
  // Keyed by device kind because the codes aren't interchangeable: a keyboard
  // binding stores a Qt::Key and a gamepad one stores a GamepadInput.
  std::map<DeviceType, std::map<ShortcutId, std::vector<InputSource>>> bindings;

  [[nodiscard]] bool appliesTo(DeviceType device) const;

  // The shipped sources for one action, in preference order — seeding keeps the
  // ones the device actually has, so a pad missing R3 falls through to the next.
  // Empty if this preset leaves the action unbound.
  [[nodiscard]] const std::vector<InputSource> &
  sourcesFor(DeviceType device, const ShortcutId &id) const;
};

// The shortcut actions the emulator supports and their shipped bindings, parsed
// from data/shortcuts.json. The app supplies the path — this library doesn't
// know where the file lives.
//
// The ShortcutEngine reads the actions; the configuration UI reads both. A
// profile's own trigger bindings live separately, on the profile.
class ShortcutRegistry {
public:
  static ShortcutRegistry &instance();

  bool loadFromFile(const std::string &path);
  bool loadFromString(const std::string &json);

  // Problems that would otherwise be invisible at runtime — an action nobody
  // declared, a code the device can't produce, a keyboard shortcut sitting on a
  // gameplay key. Empty means the file is good.
  [[nodiscard]] std::vector<std::string> validate() const;

  void registerAction(const ShortcutAction &action);
  [[nodiscard]] const ShortcutAction *getAction(const ShortcutId &id) const;
  [[nodiscard]] std::vector<ShortcutAction> listActions() const;

  [[nodiscard]] const std::vector<ShortcutPreset> &presets() const;
  [[nodiscard]] const ShortcutPreset *findPreset(const std::string &id) const;
  // The preset a new profile starts from.
  [[nodiscard]] const std::string &defaultPresetId() const;

  void clear();

private:
  std::map<ShortcutId, ShortcutAction> m_actions;
  std::vector<ShortcutPreset> m_presets;
  std::string m_defaultPresetId;
};

// Parses the form the data file uses for a trigger: "R3", or "Select+R3" where
// everything before the last '+' must be held alongside it. Returns nullopt if
// any name isn't an input this kind of device has.
std::optional<InputSource> parseInputSource(std::string_view text,
                                            DeviceType device);

} // namespace firelight::input

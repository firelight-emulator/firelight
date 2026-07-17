#pragma once
#include <firelight/input/analog_settings.hpp>
#include <firelight/input/shortcut_mapping.hpp>
#include <firelight/input/input_mapping.hpp>

#include <memory>
#include <string>
#include <vector>

namespace firelight::input {

class GamepadProfile {
public:
  explicit GamepadProfile(int id);

  [[nodiscard]] int getId() const;
  [[nodiscard]] std::string getName() const;

  bool isKeyboardProfile() const;
  void setIsKeyboardProfile(bool isKeyboardProfile);

  // Built-in profiles ship with the app and are read-only; editing one should
  // clone it to a user profile first.
  [[nodiscard]] bool isBuiltin() const;
  void setBuiltin(bool builtin);

  // The GamepadType this profile was designed for (-1 = generic / any).
  [[nodiscard]] int getBasedOnType() const;
  void setBasedOnType(int type);

  // The preset this profile's shortcuts were seeded from. Kept so the editor can
  // show which rows differ from it and reset them back — the bindings themselves
  // are the profile's own from the moment it's created, not resolved through it.
  [[nodiscard]] std::string getPresetId() const;
  void setPresetId(std::string presetId);

  [[nodiscard]] std::string getIcon() const;
  void setIcon(const std::string &icon);

  void setName(const std::string &name);

  [[nodiscard]] std::shared_ptr<InputMapping>
  getMappingForPlatformAndController(int platformId, int controllerType) const;

  void addMapping(const std::shared_ptr<InputMapping> &mapping);

  void setShortcutMapping(std::shared_ptr<ShortcutMapping> mapping);

  [[nodiscard]] std::shared_ptr<ShortcutMapping> getShortcutMapping() const;

  // Profile-wide analog defaults. A platform mapping may override these via
  // InputMapping::setAnalogOverride.
  [[nodiscard]] const AnalogSettings &getDefaultAnalogSettings() const;
  void setDefaultAnalogSettings(const AnalogSettings &settings);

  // Resolves the analog settings to use for a platform: the platform mapping's
  // override if present, otherwise the profile default.
  [[nodiscard]] AnalogSettings getAnalogSettings(int platformId,
                                                 int controllerType) const;

private:
  int m_id = -1;
  std::string m_name;

  std::vector<std::shared_ptr<InputMapping>> m_mappings;
  std::shared_ptr<ShortcutMapping> m_shortcutMapping;
  AnalogSettings m_defaultAnalogSettings;

  bool m_isKeyboardProfile = false;
  bool m_builtin = false;
  int m_basedOnType = -1;
  std::string m_icon;
  std::string m_presetId;
};

} // namespace firelight::input

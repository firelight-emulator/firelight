#pragma once
#include "input/input_sequence.hpp"

#include <functional>
#include <input/shortcuts.hpp>
#include <map>
#include <string>
#include <vector>

namespace firelight::input {

class ShortcutMapping {
public:
  ShortcutMapping(
      std::function<void(ShortcutMapping &)> syncCallback = nullptr);

  std::map<Shortcut, InputSequence> getMappings() const;

  std::string serialize() const;

  void deserialize(const std::string &data);

  void sync();

  void setMapping(Shortcut shortcut, const InputSequence &sequence);

  void clearMapping(Shortcut shortcut);

private:
  std::map<Shortcut, InputSequence> m_mappings;
  std::function<void(ShortcutMapping &)> m_syncCallback;
};

} // namespace firelight::input
#pragma once
#include <firelight/input/input_source.hpp>
#include <firelight/input/shortcut_action.hpp>

#include <cstddef>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace firelight::input {

// TODO
// A profile's shortcut triggers: for each shortcut action id, a list of input
// sources (combos) that fire it. Multiple sources per id allow alternate
// bindings on the same device. Serializes to versioned JSON
class ShortcutMapping {
public:
  explicit ShortcutMapping(
      std::function<void(ShortcutMapping &)> syncCallback = nullptr);

  [[nodiscard]] const std::vector<InputSource> &
  getBindings(const ShortcutId &id) const;
  void setBindings(const ShortcutId &id, std::vector<InputSource> sources);
  void addBinding(const ShortcutId &id, const InputSource &source);
  void removeBinding(const ShortcutId &id, std::size_t index);
  [[nodiscard]] const std::map<ShortcutId, std::vector<InputSource>> &
  getAll() const;

  [[nodiscard]] std::string serialize() const;
  void deserialize(const std::string &data);
  void sync();

private:
  std::map<ShortcutId, std::vector<InputSource>> m_bindings;
  std::function<void(ShortcutMapping &)> m_syncCallback;
};

} // namespace firelight::input

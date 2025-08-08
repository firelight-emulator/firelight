#pragma once
#include "input/input_sequence.hpp"

#include <input/shortcuts.hpp>
#include <map>
#include <string>
#include <vector>

namespace firelight::input {

class ShortcutMapping {
public:
  ShortcutMapping(int id, int profileId);

  int getId() const;
  int getProfileId() const;

  std::map<Shortcut, InputSequence> getMappings() const;

  void deserialize(const std::string &data) {
    // m_mappings.clear();
    // auto sequences = InputSequence::deserialize(data);
    // for (const auto &seq : sequences) {
    //   m_mappings[seq.shortcut] = seq;
    // }
  }

private:
  int m_id = -1;
  int m_profileId = -1;

  std::map<Shortcut, InputSequence> m_mappings;
};

} // namespace firelight::input
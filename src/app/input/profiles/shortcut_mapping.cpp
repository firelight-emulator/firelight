#include "shortcut_mapping.hpp"

namespace firelight::input {

ShortcutMapping::ShortcutMapping(
    std::function<void(ShortcutMapping &)> syncCallback)
    : m_syncCallback(std::move(syncCallback)) {}

std::map<Shortcut, InputSequence> ShortcutMapping::getMappings() const {
  return m_mappings;
}

std::string ShortcutMapping::serialize() const {
  auto result = std::string();

  // Serialization format is from:to,from:to,from:to etc
  for (const auto &[shortcut, sequence] : m_mappings) {
    result += std::to_string(shortcut) + ":";
    for (const auto &modifier : sequence.modifiers) {
      result += std::to_string(modifier) + "+";
    }
    result += std::to_string(sequence.input) + ",";
  }
  return result;
}

void ShortcutMapping::deserialize(const std::string &data) {
  m_mappings.clear();
  if (data.empty()) {
    return;
  }

  size_t start = 0;
  while (start < data.size()) {
    auto end = data.find(',', start);
    if (end == std::string::npos) {
      end = data.size();
    }

    auto mapping = data.substr(start, end - start);
    if (!mapping.empty()) {
      auto colon = mapping.find(':');
      if (colon != std::string::npos) {
        auto shortcut =
            static_cast<Shortcut>(std::stoi(mapping.substr(0, colon)));
        auto seq = mapping.substr(colon + 1);
        std::vector<int> modifiers;
        size_t plus = 0, prev = 0;
        while ((plus = seq.find('+', prev)) != std::string::npos) {
          if (plus > prev) {
            modifiers.push_back(std::stoi(seq.substr(prev, plus - prev)));
          }
          prev = plus + 1;
        }
        // The last part is the input
        int input = 0;
        if (prev < seq.size()) {
          input = std::stoi(seq.substr(prev));
        }
        InputSequence sequence;
        sequence.modifiers.clear();
        for (int mod : modifiers) {
          sequence.modifiers.push_back(static_cast<GamepadInput>(mod));
        }
        sequence.input = static_cast<GamepadInput>(input);
        m_mappings.emplace(shortcut, sequence);
      }
    }
    start = end + 1;
  }
}

void ShortcutMapping::sync() {
  if (m_syncCallback) {
    m_syncCallback(*this);
  }
}
void ShortcutMapping::setMapping(Shortcut shortcut,
                                 const InputSequence &sequence) {
  m_mappings.erase(shortcut);
  m_mappings.emplace(shortcut, sequence);
}

void ShortcutMapping::clearMapping(const Shortcut shortcut) {
  m_mappings.erase(shortcut);
}

} // namespace firelight::input
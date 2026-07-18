#pragma once

#include "protocol.hpp"

#include <cstdint>
#include <deque>
#include <string>

namespace firelight::netplay {

struct ChatMessage {
  PlayerId senderId = 0;
  std::string senderName;
  std::string text;
  int64_t timestampMs = 0;
};

class ChatLog {
public:
  explicit ChatLog(size_t maxEntries = 500) : m_maxEntries(maxEntries) {}

  void append(ChatMessage message) {
    m_entries.push_back(std::move(message));
    while (m_entries.size() > m_maxEntries) {
      m_entries.pop_front();
    }
  }

  [[nodiscard]] const std::deque<ChatMessage> &entries() const { return m_entries; }

private:
  size_t m_maxEntries;
  std::deque<ChatMessage> m_entries;
};

} // namespace firelight::netplay

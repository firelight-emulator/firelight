#pragma once

#include "lobby_backend.hpp"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace firelight::netplay {

// Signaling payloads (SDP blobs) can exceed a lobby provider's message size
// cap, so they're base64-encoded and split into chunks that ride individual
// lobby messages; the receiver reassembles by (sender, signalId)

struct SignalChunk {
  std::string signalId;
  int index = 0;
  int total = 1;
  std::string content;
};

[[nodiscard]] std::string generateSignalId();

[[nodiscard]] std::vector<SignalChunk> chunkSignal(const std::string &payload, size_t maxContentChars,
                                                   const std::string &signalId);

class SignalReassembler {
public:
  // Returns the full payload once every chunk of a signal has arrived
  std::optional<std::string> accept(PlayerId from, const SignalChunk &chunk);

private:
  struct Partial {
    int total = 0;
    std::map<int, std::string> parts;
  };

  std::map<std::pair<PlayerId, std::string>, Partial> m_partials;
};

} // namespace firelight::netplay

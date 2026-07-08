#include <firelight/netplay/base64.hpp>
#include <firelight/netplay/signal_chunker.hpp>

#include <random>

namespace firelight::netplay {

std::string generateSignalId() {
  static constexpr char HEX[] = "0123456789abcdef";
  std::random_device device;
  std::uniform_int_distribution<int> pick(0, 15);
  std::string id;
  for (int i = 0; i < 8; ++i) {
    id += HEX[pick(device)];
  }
  return id;
}

std::vector<SignalChunk> chunkSignal(const std::string &payload,
                                     const size_t maxContentChars,
                                     const std::string &signalId) {
  const auto encoded = base64EncodeText(payload);
  const size_t chunkSize = maxContentChars > 0 ? maxContentChars : 1;
  const int total =
      static_cast<int>((encoded.size() + chunkSize - 1) / chunkSize);

  std::vector<SignalChunk> chunks;
  for (int i = 0; i < total; ++i) {
    chunks.push_back(SignalChunk{
        .signalId = signalId,
        .index = i,
        .total = total,
        .content = encoded.substr(static_cast<size_t>(i) * chunkSize,
                                  chunkSize),
    });
  }
  if (chunks.empty()) {
    chunks.push_back(
        SignalChunk{.signalId = signalId, .index = 0, .total = 1});
  }
  return chunks;
}

std::optional<std::string>
SignalReassembler::accept(const PlayerId from, const SignalChunk &chunk) {
  if (chunk.total < 1 || chunk.index < 0 || chunk.index >= chunk.total) {
    return std::nullopt;
  }

  // Keep abandoned partial signals from accumulating.
  if (m_partials.size() > 32) {
    m_partials.clear();
  }

  auto &partial = m_partials[{from, chunk.signalId}];
  partial.total = chunk.total;
  partial.parts[chunk.index] = chunk.content;
  if (static_cast<int>(partial.parts.size()) < partial.total) {
    return std::nullopt;
  }

  std::string encoded;
  for (const auto &[index, content] : partial.parts) {
    encoded += content;
  }
  m_partials.erase({from, chunk.signalId});
  return base64DecodeText(encoded);
}

} // namespace firelight::netplay

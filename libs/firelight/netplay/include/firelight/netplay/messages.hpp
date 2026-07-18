#pragma once

#include "protocol.hpp"
#include "session_descriptor.hpp"
#include "session_phase.hpp"
#include "slot_table.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <variant>

namespace firelight::netplay {

// Control-channel messages (JSON on the wire, host is authoritative)

struct Hello {
  int proto = 0;
  std::string appVersion;
  PlayerId memberId = 0;
};

// Everything the guest's audio/video decoder needs to start mid-stream
struct StreamConfig {
  std::string codec = "h264";
  std::string extradataB64;
  int width = 0;
  int height = 0;
  int fps = 60;
  std::string audioCodec = "opus";
  int sampleRate = 48000;
  int channels = 2;
  int inputTickHz = 60;

  bool operator==(const StreamConfig &) const = default;
};

// Full state snapshot sent to any member whose control channel opens — a fresh
// join, a mid-game join, or a reconnect all recover from this one message
// (Fields avoid the name `slots`: Qt defines it as a macro and these headers
// are included from Qt code.)
struct Welcome {
  int proto = 0;
  GamePhase phase = GamePhase::Idle;
  SlotTable table;
  std::optional<SessionDescriptor> game;
  std::optional<StreamConfig> streamConfig;
};

struct Reject {
  std::string code;
  std::string message;
};

struct SlotTableMessage {
  SlotTable table;
};

struct GameSelected {
  SessionDescriptor game;
};

struct ReadyState {
  bool ready = false;
};

struct SessionStarting {};

struct StreamStart {};

struct PauseState {
  bool paused = false;
};

struct SessionEnded {
  std::string reason;
};

struct Ping {
  int64_t t = 0;
};

struct Pong {
  int64_t t = 0;
};

using ControlMessage =
    std::variant<Hello, Welcome, Reject, SlotTableMessage, GameSelected,
                 ReadyState, SessionStarting, StreamConfig, StreamStart,
                 PauseState, SessionEnded, Ping, Pong>;

[[nodiscard]] std::string encodeMessage(const ControlMessage &message);
[[nodiscard]] std::optional<ControlMessage>
decodeMessage(const std::string &text);

} // namespace firelight::netplay

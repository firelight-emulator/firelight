#include <firelight/netplay/messages.hpp>
#include <firelight/netplay/protocol.hpp>

#include <nlohmann/json.hpp>

namespace firelight::netplay {

namespace {
using nlohmann::json;

json toJson(const SlotTable &slots) {
  json arr = json::array();
  for (const auto &slot : slots.all()) {
    if (!slot) {
      arr.push_back(nullptr);
      continue;
    }
    arr.push_back({{"memberId", slot->memberId},
                   {"localPadIndex", slot->localPadIndex},
                   {"displayName", slot->displayName},
                   {"ready", slot->ready}});
  }
  return arr;
}

SlotTable slotTableFromJson(const json &arr) {
  SlotTable table;
  if (!arr.is_array()) {
    return table;
  }
  for (int i = 0; i < static_cast<int>(arr.size()) && i < MAX_SLOTS; ++i) {
    const auto &entry = arr[i];
    if (entry.is_null()) {
      continue;
    }
    table.assign(i, SlotAssignment{
                        .memberId = entry.value("memberId", PlayerId{0}),
                        .localPadIndex = entry.value("localPadIndex", 0),
                        .displayName = entry.value("displayName", ""),
                        .ready = entry.value("ready", false),
                    });
  }
  return table;
}

json toJson(const SessionDescriptor &game) {
  return {{"gameName", game.gameName},
          {"contentHash", game.contentHash},
          {"platformId", game.platformId},
          {"artUrl", game.artUrl},
          {"strategy", game.strategy == SyncStrategyKind::Lockstep
                           ? "lockstep"
                           : "host-stream"}};
}

SessionDescriptor gameFromJson(const json &obj) {
  return SessionDescriptor{
      .gameName = obj.value("gameName", ""),
      .contentHash = obj.value("contentHash", ""),
      .platformId = obj.value("platformId", 0),
      .artUrl = obj.value("artUrl", ""),
      .strategy = obj.value("strategy", "host-stream") == "lockstep"
                      ? SyncStrategyKind::Lockstep
                      : SyncStrategyKind::HostStream,
  };
}

json toJson(const StreamConfig &config) {
  return {{"codec", config.codec},
          {"extradataB64", config.extradataB64},
          {"width", config.width},
          {"height", config.height},
          {"fps", config.fps},
          {"audioCodec", config.audioCodec},
          {"sampleRate", config.sampleRate},
          {"channels", config.channels},
          {"inputTickHz", config.inputTickHz}};
}

StreamConfig streamConfigFromJson(const json &obj) {
  return StreamConfig{
      .codec = obj.value("codec", "h264"),
      .extradataB64 = obj.value("extradataB64", ""),
      .width = obj.value("width", 0),
      .height = obj.value("height", 0),
      .fps = obj.value("fps", 60),
      .audioCodec = obj.value("audioCodec", "opus"),
      .sampleRate = obj.value("sampleRate", 48000),
      .channels = obj.value("channels", 2),
      .inputTickHz = obj.value("inputTickHz", 60),
  };
}

const char *phaseName(const GamePhase phase) {
  switch (phase) {
  case GamePhase::Starting:
    return "starting";
  case GamePhase::InGame:
    return "in-game";
  case GamePhase::Idle:
  default:
    return "idle";
  }
}

GamePhase phaseFromName(const std::string &name) {
  if (name == "starting") {
    return GamePhase::Starting;
  }
  if (name == "in-game") {
    return GamePhase::InGame;
  }
  return GamePhase::Idle;
}
} // namespace

std::string encodeMessage(const ControlMessage &message) {
  json out;
  std::visit(
      [&out](const auto &m) {
        using T = std::decay_t<decltype(m)>;
        if constexpr (std::is_same_v<T, Hello>) {
          out = {{"type", "hello"},
                 {"proto", m.proto},
                 {"appVersion", m.appVersion},
                 {"memberId", m.memberId}};
        } else if constexpr (std::is_same_v<T, Welcome>) {
          out = {{"type", "welcome"},
                 {"proto", m.proto},
                 {"phase", phaseName(m.phase)},
                 {"slots", toJson(m.table)}};
          if (m.game) {
            out["game"] = toJson(*m.game);
          }
          if (m.streamConfig) {
            out["streamConfig"] = toJson(*m.streamConfig);
          }
        } else if constexpr (std::is_same_v<T, Reject>) {
          out = {{"type", "reject"}, {"code", m.code}, {"message", m.message}};
        } else if constexpr (std::is_same_v<T, SlotTableMessage>) {
          out = {{"type", "slots"}, {"slots", toJson(m.table)}};
        } else if constexpr (std::is_same_v<T, GameSelected>) {
          out = {{"type", "gameSelected"}, {"game", toJson(m.game)}};
        } else if constexpr (std::is_same_v<T, ReadyState>) {
          out = {{"type", "ready"}, {"ready", m.ready}};
        } else if constexpr (std::is_same_v<T, SessionStarting>) {
          out = {{"type", "sessionStarting"}};
        } else if constexpr (std::is_same_v<T, StreamConfig>) {
          out = toJson(m);
          out["type"] = "streamConfig";
        } else if constexpr (std::is_same_v<T, StreamStart>) {
          out = {{"type", "streamStart"}};
        } else if constexpr (std::is_same_v<T, PauseState>) {
          out = {{"type", "pause"}, {"paused", m.paused}};
        } else if constexpr (std::is_same_v<T, SessionEnded>) {
          out = {{"type", "sessionEnded"}, {"reason", m.reason}};
        } else if constexpr (std::is_same_v<T, Ping>) {
          out = {{"type", "ping"}, {"t", m.t}};
        } else if constexpr (std::is_same_v<T, Pong>) {
          out = {{"type", "pong"}, {"t", m.t}};
        }
      },
      message);
  return out.dump();
}

std::optional<ControlMessage> decodeMessage(const std::string &text) {
  const auto parsed = json::parse(text, nullptr, false);
  if (parsed.is_discarded() || !parsed.is_object()) {
    return std::nullopt;
  }
  const auto type = parsed.value("type", "");

  if (type == "hello") {
    return Hello{.proto = parsed.value("proto", 0),
                 .appVersion = parsed.value("appVersion", ""),
                 .memberId = parsed.value("memberId", PlayerId{0})};
  }
  if (type == "welcome") {
    Welcome welcome{.proto = parsed.value("proto", 0),
                    .phase = phaseFromName(parsed.value("phase", "idle")),
                    .table = slotTableFromJson(parsed.value("slots", json()))};
    if (parsed.contains("game")) {
      welcome.game = gameFromJson(parsed["game"]);
    }
    if (parsed.contains("streamConfig")) {
      welcome.streamConfig = streamConfigFromJson(parsed["streamConfig"]);
    }
    return welcome;
  }
  if (type == "reject") {
    return Reject{.code = parsed.value("code", ""),
                  .message = parsed.value("message", "")};
  }
  if (type == "slots") {
    return SlotTableMessage{
        .table = slotTableFromJson(parsed.value("slots", json()))};
  }
  if (type == "gameSelected") {
    return GameSelected{.game = gameFromJson(parsed.value("game", json()))};
  }
  if (type == "ready") {
    return ReadyState{.ready = parsed.value("ready", false)};
  }
  if (type == "sessionStarting") {
    return SessionStarting{};
  }
  if (type == "streamConfig") {
    return streamConfigFromJson(parsed);
  }
  if (type == "streamStart") {
    return StreamStart{};
  }
  if (type == "pause") {
    return PauseState{.paused = parsed.value("paused", false)};
  }
  if (type == "sessionEnded") {
    return SessionEnded{.reason = parsed.value("reason", "")};
  }
  if (type == "ping") {
    return Ping{.t = parsed.value("t", int64_t{0})};
  }
  if (type == "pong") {
    return Pong{.t = parsed.value("t", int64_t{0})};
  }
  return std::nullopt;
}

} // namespace firelight::netplay

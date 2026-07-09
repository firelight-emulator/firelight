#pragma once
#include <firelight/input/input_source.hpp>

#include <nlohmann/json.hpp>

namespace firelight::input {

struct TurboOptions {
  bool enabled = false;
  float rateHz = 10.0f; // presses per second while the source is held

  bool operator==(const TurboOptions &o) const {
    return enabled == o.enabled && rateHz == o.rateHz;
  }
};

inline void to_json(nlohmann::json &j, const TurboOptions &t) {
  j = nlohmann::json{{"enabled", t.enabled}, {"rateHz", t.rateHz}};
}

inline void from_json(const nlohmann::json &j, TurboOptions &t) {
  t.enabled = j.value("enabled", false);
  t.rateHz = j.value("rateHz", 10.0f);
}

// A single physical source bound to an emulated input, plus how it behaves.
// Multiple Bindings may target the same emulated input; their evaluated results
// are OR'd together (digital) or summed/maxed (analog).
struct Binding {
  InputSource source;
  bool toggle = false;    // press latches the emulated input on/off
  TurboOptions turbo;     // autofire while the source is held
  float threshold = 0.5f; // analog source -> digital target (such as stick pushed halfway to trigger button press)
  bool invert = false;    // axis source -> axis target
  float scale = 1.0f;     // axis source -> axis target sensitivity

  bool operator==(const Binding &o) const {
    return source == o.source && toggle == o.toggle && turbo == o.turbo &&
           threshold == o.threshold && invert == o.invert && scale == o.scale;
  }
};

inline void to_json(nlohmann::json &j, const Binding &b) {
  j = nlohmann::json{{"source", b.source}};
  // Only emit non-default options to keep the serialized form compact.
  if (b.toggle) {
    j["toggle"] = b.toggle;
  }
  if (b.turbo.enabled) {
    j["turbo"] = b.turbo;
  }
  if (b.threshold != 0.5f) {
    j["threshold"] = b.threshold;
  }
  if (b.invert) {
    j["invert"] = b.invert;
  }
  if (b.scale != 1.0f) {
    j["scale"] = b.scale;
  }
}

inline void from_json(const nlohmann::json &j, Binding &b) {
  j.at("source").get_to(b.source);
  b.toggle = j.value("toggle", false);
  if (j.contains("turbo")) {
    j.at("turbo").get_to(b.turbo);
  }
  b.threshold = j.value("threshold", 0.5f);
  b.invert = j.value("invert", false);
  b.scale = j.value("scale", 1.0f);
}

} // namespace firelight::input

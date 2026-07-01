#pragma once
#include <nlohmann/json.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace firelight::input {

enum class ResponseCurve { Linear, Exponential };

NLOHMANN_JSON_SERIALIZE_ENUM(ResponseCurve,
                             {
                                 {ResponseCurve::Linear, "linear"},
                                 {ResponseCurve::Exponential, "exponential"},
                             })

// Tuning for one analog stick axis. Fractions are of full range (0..1). The
// defaults reproduce the previous hardcoded behavior: an inner deadzone of
// 8192/32767 (~0.25) with everything else identity.
struct AxisSettings {
  float innerDeadzone = 0.25f;
  float outerDeadzone = 0.0f;
  float sensitivity = 1.0f;
  float antiDeadzone = 0.0f;
  ResponseCurve curve = ResponseCurve::Linear;
  float curveExponent = 1.0f;

  bool operator==(const AxisSettings &o) const {
    return innerDeadzone == o.innerDeadzone && outerDeadzone == o.outerDeadzone &&
           sensitivity == o.sensitivity && antiDeadzone == o.antiDeadzone &&
           curve == o.curve && curveExponent == o.curveExponent;
  }
};

inline void to_json(nlohmann::json &j, const AxisSettings &a) {
  j = nlohmann::json{{"innerDeadzone", a.innerDeadzone},
                     {"outerDeadzone", a.outerDeadzone},
                     {"sensitivity", a.sensitivity},
                     {"antiDeadzone", a.antiDeadzone},
                     {"curve", a.curve},
                     {"curveExponent", a.curveExponent}};
}

inline void from_json(const nlohmann::json &j, AxisSettings &a) {
  a.innerDeadzone = j.value("innerDeadzone", 0.25f);
  a.outerDeadzone = j.value("outerDeadzone", 0.0f);
  a.sensitivity = j.value("sensitivity", 1.0f);
  a.antiDeadzone = j.value("antiDeadzone", 0.0f);
  a.curve = j.value("curve", ResponseCurve::Linear);
  a.curveExponent = j.value("curveExponent", 1.0f);
}

struct TriggerSettings {
  float deadzone = 0.0f;
  float threshold = 0.5f; // fraction of full range to count as "pressed"

  bool operator==(const TriggerSettings &o) const {
    return deadzone == o.deadzone && threshold == o.threshold;
  }
};

inline void to_json(nlohmann::json &j, const TriggerSettings &t) {
  j = nlohmann::json{{"deadzone", t.deadzone}, {"threshold", t.threshold}};
}

inline void from_json(const nlohmann::json &j, TriggerSettings &t) {
  t.deadzone = j.value("deadzone", 0.0f);
  t.threshold = j.value("threshold", 0.5f);
}

struct AnalogSettings {
  AxisSettings leftStick;
  AxisSettings rightStick;
  TriggerSettings leftTrigger;
  TriggerSettings rightTrigger;

  bool operator==(const AnalogSettings &o) const {
    return leftStick == o.leftStick && rightStick == o.rightStick &&
           leftTrigger == o.leftTrigger && rightTrigger == o.rightTrigger;
  }
};

inline void to_json(nlohmann::json &j, const AnalogSettings &a) {
  j = nlohmann::json{{"leftStick", a.leftStick},
                     {"rightStick", a.rightStick},
                     {"leftTrigger", a.leftTrigger},
                     {"rightTrigger", a.rightTrigger}};
}

inline void from_json(const nlohmann::json &j, AnalogSettings &a) {
  if (j.contains("leftStick")) {
    j.at("leftStick").get_to(a.leftStick);
  }
  if (j.contains("rightStick")) {
    j.at("rightStick").get_to(a.rightStick);
  }
  if (j.contains("leftTrigger")) {
    j.at("leftTrigger").get_to(a.leftTrigger);
  }
  if (j.contains("rightTrigger")) {
    j.at("rightTrigger").get_to(a.rightTrigger);
  }
}

// Maps a raw SDL axis value (-32768..32767) through the given settings and
// returns a processed value in the same range. Each axis is processed as an
// independent scalar, matching the previous per-axis deadzone behavior.
inline int16_t applyAxisSettings(const int rawValue, const AxisSettings &s) {
  constexpr float kMax = 32767.0f;
  const float n = std::clamp(rawValue / kMax, -1.0f, 1.0f);
  const float sign = n < 0.0f ? -1.0f : 1.0f;
  float m = std::fabs(n);

  const float lower = std::clamp(s.innerDeadzone, 0.0f, 1.0f);
  const float upper = std::clamp(1.0f - s.outerDeadzone, lower + 1e-4f, 1.0f);
  if (m <= lower) {
    return 0;
  }
  m = std::clamp((m - lower) / (upper - lower), 0.0f, 1.0f);

  if (s.curve == ResponseCurve::Exponential && s.curveExponent > 0.0f) {
    m = std::pow(m, s.curveExponent);
  }

  if (s.antiDeadzone > 0.0f) {
    m = s.antiDeadzone + m * (1.0f - s.antiDeadzone);
  }

  m = std::clamp(m * s.sensitivity, 0.0f, 1.0f);

  return static_cast<int16_t>(sign * m * kMax);
}

} // namespace firelight::input

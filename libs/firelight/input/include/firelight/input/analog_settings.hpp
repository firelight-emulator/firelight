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

  // Maps a raw SDL axis value (-32768..32767) through these settings and returns
  // a processed value in the same range. Each axis is processed as an
  // independent scalar, matching the previous per-axis deadzone behavior.
  [[nodiscard]] int16_t apply(const int rawValue) const {
    constexpr float AXIS_MAX = 32767.0f;
    // Smallest allowed gap between the inner and outer bounds, so the rescale
    // below never divides by (near) zero.
    constexpr float MIN_DEADZONE_SPAN = 1e-4f;

    const float normalized = std::clamp(rawValue / AXIS_MAX, -1.0f, 1.0f);
    const float sign = normalized < 0.0f ? -1.0f : 1.0f;
    float magnitude = std::fabs(normalized);

    const float innerBound = std::clamp(innerDeadzone, 0.0f, 1.0f);
    const float outerBound =
        std::clamp(1.0f - outerDeadzone, innerBound + MIN_DEADZONE_SPAN, 1.0f);
    if (magnitude <= innerBound) {
      return 0;
    }
    // Rescale the live range (innerBound..outerBound) back to a full 0..1.
    magnitude = std::clamp((magnitude - innerBound) / (outerBound - innerBound),
                           0.0f, 1.0f);

    if (curve == ResponseCurve::Exponential && curveExponent > 0.0f) {
      magnitude = std::pow(magnitude, curveExponent);
    }

    if (antiDeadzone > 0.0f) {
      magnitude = antiDeadzone + magnitude * (1.0f - antiDeadzone);
    }

    magnitude = std::clamp(magnitude * sensitivity, 0.0f, 1.0f);

    return static_cast<int16_t>(sign * magnitude * AXIS_MAX);
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

} // namespace firelight::input

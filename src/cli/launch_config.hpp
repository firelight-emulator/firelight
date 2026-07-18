#pragma once

#include <firelight/input/gamepad_type.hpp>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace firelight::cli {

// A single key=value emulation-setting override parsed from the command line
using OverridePair = std::pair<std::string, std::string>;

// Loads bulk emulation overrides from a file for `--settings-file`. The format
// is auto-detected from the contents: if they begin with `{` the file is parsed
// as a JSON object (`{"key": value, ...}`; string/bool/number values are
// stringified), otherwise as a Java-style properties file (one `key=value` per
// line; blank lines and `#`/`;` comment lines are ignored; whitespace around the
// key and value is trimmed). Throws std::runtime_error if the file cannot be
// read or is malformed
std::vector<OverridePair> loadOverrideFile(const std::string &path);

// Maps a friendly controller name from `--controller` (case-insensitive, e.g.
// "xbox360", "dualshock4", "switch-pro") to a GamepadType, or nullopt if the
// name is not recognized
std::optional<GamepadType> parseControllerType(const std::string &name);

// The controller names accepted by parseControllerType, for help/error text
std::string controllerTypeNames();

} // namespace firelight::cli

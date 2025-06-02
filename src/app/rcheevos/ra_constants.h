#pragma once

#include <string>

namespace firelight::achievements {
constexpr int UNSUPPORTED_EMULATOR_ACHIEVEMENT_ID = 101000001;
const std::string RA_DOREQUEST_URL =
    "https://retroachievements.org/dorequest.php";
const std::string USER_AGENT = "Firelight/0.9.5";
const std::string OFFLINE_USER_AGENT = USER_AGENT + " [offline]";
} // namespace firelight::achievements
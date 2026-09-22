#pragma once

#include <string>
#include <string_view>

namespace firelight {

/**
 * The canonical region and language vocabulary
 */
namespace regions {
inline constexpr auto US = "US";
inline constexpr auto EU = "EU";
inline constexpr auto JP = "JP";
inline constexpr auto WORLD = "WORLD";
inline constexpr auto ASIA = "ASIA";
inline constexpr auto AU = "AU";
inline constexpr auto BR = "BR";
inline constexpr auto CA = "CA";
inline constexpr auto CN = "CN";
inline constexpr auto KR = "KR";
inline constexpr auto TW = "TW";
inline constexpr auto HK = "HK";
inline constexpr auto FR = "FR";
inline constexpr auto DE = "DE";
inline constexpr auto IT = "IT";
inline constexpr auto ES = "ES";
inline constexpr auto NL = "NL";
inline constexpr auto SE = "SE";
inline constexpr auto NO = "NO";
inline constexpr auto DK = "DK";
inline constexpr auto FI = "FI";
inline constexpr auto RU = "RU";
inline constexpr auto IN = "IN";
inline constexpr auto GB = "GB";
inline constexpr auto UNKNOWN = "UNKNOWN";
} // namespace regions

/**
 * The region a filename tag names, or empty when it names no region
 *
 * Accepts full names ("Europe"), codes ("EU") and the GoodTools single letters ("E"). Case insensitive
 */
[[nodiscard]] std::string regionForTag(std::string_view tag);

/**
 * The ISO 639-1 code a tag names, or empty when it names no language
 *
 * Accepts the two-letter code ("En") and the full English name ("English"). Case insensitive
 */
[[nodiscard]] std::string languageForTag(std::string_view tag);

/**
 * @return Whether the code is recognized
 */
[[nodiscard]] bool isKnownRegion(std::string_view code);

/**
 * @return Whether the code is recognized
 */
[[nodiscard]] bool isKnownLanguage(std::string_view code);

} // namespace firelight

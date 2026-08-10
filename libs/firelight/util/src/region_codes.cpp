#include <firelight/util/region_codes.hpp>

#include <firelight/util/strings.hpp>

#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <unordered_set>

namespace firelight {

namespace {
// Full names, ISO codes and the GoodTools single letters, all keyed lowercase. A token naming both a
// region and a language (Norway/Norwegian) appears in both tables; the caller decides which reading
// applies
const std::unordered_map<std::string, std::string> &regionTable() {
  static const std::unordered_map<std::string, std::string> table = {
      {"usa", regions::US},         {"us", regions::US},
      {"u", regions::US},           {"america", regions::US},
      {"europe", regions::EU},      {"eu", regions::EU},
      {"e", regions::EU},           {"japan", regions::JP},
      {"jp", regions::JP},          {"j", regions::JP},
      {"world", regions::WORLD},    {"w", regions::WORLD},
      {"asia", regions::ASIA},      {"australia", regions::AU},
      {"au", regions::AU},          {"a", regions::AU},
      {"brazil", regions::BR},      {"br", regions::BR},
      {"b", regions::BR},           {"canada", regions::CA},
      {"ca", regions::CA},          {"china", regions::CN},
      {"cn", regions::CN},          {"c", regions::CN},
      {"korea", regions::KR},       {"kr", regions::KR},
      {"k", regions::KR},           {"taiwan", regions::TW},
      {"tw", regions::TW},          {"hong kong", regions::HK},
      {"hk", regions::HK},          {"france", regions::FR},
      {"fr", regions::FR},          {"f", regions::FR},
      {"germany", regions::DE},     {"de", regions::DE},
      {"g", regions::DE},           {"italy", regions::IT},
      {"it", regions::IT},          {"i", regions::IT},
      {"spain", regions::ES},       {"es", regions::ES},
      {"s", regions::ES},           {"netherlands", regions::NL},
      {"nl", regions::NL},          {"sweden", regions::SE},
      {"se", regions::SE},          {"norway", regions::NO},
      {"no", regions::NO},          {"denmark", regions::DK},
      {"dk", regions::DK},          {"finland", regions::FI},
      {"fi", regions::FI},          {"russia", regions::RU},
      {"ru", regions::RU},          {"india", regions::IN},
      {"in", regions::IN},          {"uk", regions::GB},
      {"gb", regions::GB},          {"england", regions::GB},
      {"unknown", regions::UNKNOWN}};
  return table;
}

const std::unordered_map<std::string, std::string> &languageTable() {
  static const std::unordered_map<std::string, std::string> table = {
      {"en", "en"},         {"english", "en"},    {"fr", "fr"},        {"french", "fr"},  {"de", "de"},
      {"german", "de"},     {"es", "es"},         {"spanish", "es"},   {"it", "it"},      {"italian", "it"},
      {"ja", "ja"},         {"japanese", "ja"},   {"nl", "nl"},        {"dutch", "nl"},   {"pt", "pt"},
      {"portuguese", "pt"}, {"sv", "sv"},         {"swedish", "sv"},   {"no", "no"},      {"norwegian", "no"},
      {"da", "da"},         {"danish", "da"},     {"fi", "fi"},        {"finnish", "fi"}, {"zh", "zh"},
      {"chinese", "zh"},    {"ko", "ko"},         {"korean", "ko"},    {"ru", "ru"},      {"russian", "ru"},
      {"pl", "pl"},         {"polish", "pl"}};
  return table;
}
} // namespace

std::string regionForTag(const std::string_view tag) {
  const auto &table = regionTable();
  const auto it = table.find(strings::toLower(tag));
  return it != table.end() ? it->second : std::string{};
}

std::string languageForTag(const std::string_view tag) {
  const auto &table = languageTable();
  const auto it = table.find(strings::toLower(tag));
  return it != table.end() ? it->second : std::string{};
}

bool isKnownRegion(const std::string_view code) {
  const auto resolved = regionForTag(code);
  return !resolved.empty() && resolved == std::string(code);
}

bool isKnownLanguage(const std::string_view code) {
  const auto resolved = languageForTag(code);
  return !resolved.empty() && resolved == std::string(code);
}

} // namespace firelight

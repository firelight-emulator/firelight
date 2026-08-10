#include <firelight/library/filename_tags.hpp>

#include <firelight/util/region_codes.hpp>
#include <firelight/util/strings.hpp>

#include <algorithm>
#include <cctype>

namespace firelight::library {

namespace {
// The number trailing a keyword, as in "Disc 2"; 0 when there is none
int trailingNumber(const std::string &value, const size_t afterKeyword) {
  const auto digits = strings::trim(value.substr(afterKeyword));

  if (digits.empty() || !std::all_of(digits.begin(), digits.end(), [](const unsigned char c) {
        return std::isdigit(c);
      })) {
    return 0;
  }

  return std::stoi(digits);
}

// The single-token groups that name something other than a region or a language
bool applyKeywordGroup(const std::string &token, FilenameTags &tags) {
  if (strings::startsWithIgnoringCase(token, "rev ")) {
    tags.revision = strings::trim(token.substr(4));
    return true;
  }

  if (token.size() > 1 && (token[0] == 'v' || token[0] == 'V') && std::isdigit(static_cast<unsigned char>(token[1]))) {
    tags.version = token.substr(1);
    return true;
  }

  for (const auto *keyword : {"disc ", "disk ", "cd "}) {
    if (strings::startsWithIgnoringCase(token, keyword)) {
      const auto number = trailingNumber(token, std::string(keyword).size());

      if (number > 0) {
        tags.discNumber = number;
        return true;
      }
    }
  }

  const auto lowered = strings::toLower(token);

  if (strings::startsWithIgnoringCase(token, "beta")) {
    tags.flags.emplace_back(content_flags::BETA);
    return true;
  }

  if (lowered == "proto" || lowered == "prototype") {
    tags.flags.emplace_back(content_flags::PROTO);
    return true;
  }

  if (lowered == "demo") {
    tags.flags.emplace_back(content_flags::DEMO);
    return true;
  }

  if (lowered == "sample") {
    tags.flags.emplace_back(content_flags::SAMPLE);
    return true;
  }

  if (lowered == "unl" || lowered == "unlicensed") {
    tags.flags.emplace_back(content_flags::UNLICENSED);
    return true;
  }

  if (lowered == "alt") {
    tags.flags.emplace_back(content_flags::ALT);
    return true;
  }

  return false;
}

void applyParenGroup(const std::string &body, FilenameTags &tags) {
  const auto tokens = strings::split(body, ',');

  if (tokens.empty()) {
    return;
  }

  if (tokens.size() == 1 && applyKeywordGroup(tokens.front(), tags)) {
    return;
  }

  // A group of two-letter codes is a language list. That is what decides tokens naming both a
  // region and a language, so (No) is Norwegian rather than Norway
  const auto isLanguageGroup = std::all_of(tokens.begin(), tokens.end(), [](const std::string &token) {
    return token.size() == 2 && !languageForTag(token).empty();
  });

  if (isLanguageGroup) {
    for (const auto &token : tokens) {
      tags.languages.push_back(languageForTag(token));
    }

    return;
  }

  const auto isRegionGroup = std::all_of(tokens.begin(), tokens.end(), [](const std::string &token) {
    return !regionForTag(token).empty();
  });

  if (isRegionGroup) {
    for (const auto &token : tokens) {
      tags.regions.push_back(regionForTag(token));
    }

    return;
  }

  for (const auto &token : tokens) {
    applyKeywordGroup(token, tags);
  }
}

void applyBracketGroup(const std::string &body, FilenameTags &tags) {
  const auto trimmed = strings::trim(body);

  if (trimmed.empty()) {
    return;
  }

  if (trimmed.size() > 2 && (trimmed[0] == 'T' || trimmed[0] == 't') && (trimmed[1] == '+' || trimmed[1] == '-')) {
    tags.isTranslation = true;
    tags.translationLanguage = languageForTag(trimmed.substr(2, 2));
    tags.flags.emplace_back(content_flags::TRANSLATION);
    return;
  }

  if (trimmed == "!") {
    tags.flags.emplace_back(content_flags::VERIFIED_DUMP);
    return;
  }

  // The dump flags carry an optional index, as in [b2]
  switch (std::tolower(static_cast<unsigned char>(trimmed[0]))) {
  case 'b':
    tags.flags.emplace_back(content_flags::BAD_DUMP);
    return;
  case 'a':
    tags.flags.emplace_back(content_flags::ALT);
    return;
  case 'h':
    tags.flags.emplace_back(content_flags::HACK);
    return;
  case 'p':
    tags.flags.emplace_back(content_flags::PIRATE);
    return;
  case 'o':
    tags.flags.emplace_back(content_flags::OVERDUMP);
    return;
  case 't':
    tags.flags.emplace_back(content_flags::TRAINED);
    return;
  default:
    return;
  }
}
} // namespace

FilenameTags parseFilenameTags(const std::string_view fileName) {
  FilenameTags tags;

  std::string stem(fileName);

  const auto separator = stem.find_last_of("/\\");

  if (separator != std::string::npos) {
    stem = stem.substr(separator + 1);
  }

  const auto dot = stem.find_last_of('.');

  if (dot != std::string::npos && dot > 0) {
    stem = stem.substr(0, dot);
  }

  // TODO
  // Everything the tag groups are not. Taking only what comes before the first group would
  // drop a suffix like "-patched", which is the whole of what tells that file apart from
  // the dump it was made from
  std::string titleText;

  for (size_t i = 0; i < stem.size(); ++i) {
    const auto opener = stem[i];

    if (opener != '(' && opener != '[') {
      titleText.push_back(stem[i]);
      continue;
    }

    const auto closer = opener == '(' ? ')' : ']';
    const auto end = stem.find(closer, i + 1);

    if (end == std::string::npos) {
      titleText.append(stem.substr(i));
      break;
    }

    // TODO
    // The space that stood in front of the group goes with it, so "World (USA)-patched"
    // closes up to "World-patched" rather than keeping a gap where the tag used to be
    while (!titleText.empty() && std::isspace(static_cast<unsigned char>(titleText.back())) != 0) {
      titleText.pop_back();
    }

    const auto body = stem.substr(i + 1, end - i - 1);

    if (opener == '(') {
      applyParenGroup(body, tags);
    } else {
      applyBracketGroup(body, tags);
    }

    i = end;
  }

  tags.title = strings::collapseWhitespace(titleText);

  // TODO
  // Set dumps number their files, as in "0028 - Kirby - Canvas Curse". The leading zero
  // is what separates an index from a title that begins with a number, so "1943 - The
  // Battle of Midway" and "007 James Bond" keep theirs
  if (tags.title.size() > 1 && tags.title[0] == '0') {
    auto digits = size_t{0};

    while (digits < tags.title.size() && std::isdigit(static_cast<unsigned char>(tags.title[digits]))) {
      ++digits;
    }

    const auto rest = strings::trim(tags.title.substr(digits));

    if (digits > 1 && rest.size() > 1 && rest[0] == '-') {
      tags.title = strings::trim(rest.substr(1));
    }
  }

  return tags;
}

std::string normalizeTitle(const std::string &title) {
  return strings::collapseWhitespace(strings::toLower(title));
}

} // namespace firelight::library

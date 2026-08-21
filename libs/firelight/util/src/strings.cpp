#include <firelight/util/strings.hpp>

#include <algorithm>
#include <array>
#include <cctype>

namespace firelight::strings {

namespace {
bool isSpace(const char character) { return std::isspace(static_cast<unsigned char>(character)) != 0; }

bool isAlphanumeric(const char character) { return std::isalnum(static_cast<unsigned char>(character)) != 0; }

char lower(const char character) { return static_cast<char>(std::tolower(static_cast<unsigned char>(character))); }

// Numerals past 20 are rare enough in titles that spelling them out beats parsing them
constexpr std::array<std::pair<std::string_view, std::string_view>, 20> ROMAN_NUMERALS = {{
    {"xx", "20"},   {"xix", "19"}, {"xviii", "18"}, {"xvii", "17"}, {"xvi", "16"}, {"xv", "15"},  {"xiv", "14"},
    {"xiii", "13"}, {"xii", "12"}, {"xi", "11"},    {"x", "10"},    {"ix", "9"},   {"viii", "8"}, {"vii", "7"},
    {"vi", "6"},    {"v", "5"},    {"iv", "4"},     {"iii", "3"},   {"ii", "2"},   {"i", "1"},
}};

constexpr std::array<std::string_view, 3> ARTICLES = {"the", "a", "an"};
} // namespace

std::string toLower(const std::string_view value) {
  std::string lowered;
  lowered.reserve(value.size());

  for (const auto character : value) {
    lowered.push_back(lower(character));
  }

  return lowered;
}

std::string trim(const std::string_view value) {
  auto first = value.begin();
  auto last = value.end();

  while (first != last && isSpace(*first)) {
    ++first;
  }

  while (last != first && isSpace(*(last - 1))) {
    --last;
  }

  return std::string(first, last);
}

std::string collapseWhitespace(const std::string_view value) {
  std::string collapsed;
  collapsed.reserve(value.size());

  auto pendingSpace = false;

  for (const auto character : value) {
    if (isSpace(character)) {
      pendingSpace = !collapsed.empty();
      continue;
    }

    if (pendingSpace) {
      collapsed.push_back(' ');
      pendingSpace = false;
    }

    collapsed.push_back(character);
  }

  return collapsed;
}

std::string replaceAll(const std::string_view value, const std::string_view from, const std::string_view to) {
  if (from.empty()) {
    return std::string(value);
  }

  std::string replaced;
  replaced.reserve(value.size());

  std::size_t position = 0;

  while (position < value.size()) {
    const auto found = value.find(from, position);

    if (found == std::string_view::npos) {
      replaced.append(value.substr(position));
      break;
    }

    replaced.append(value.substr(position, found - position));
    replaced.append(to);
    position = found + from.size();
  }

  return replaced;
}

std::vector<std::string> split(const std::string_view value, const char separator, const bool keepEmpty) {
  std::vector<std::string> pieces;
  std::size_t position = 0;

  while (true) {
    const auto found = value.find(separator, position);
    const auto piece =
        trim(value.substr(position, found == std::string_view::npos ? std::string_view::npos : found - position));

    if (keepEmpty || !piece.empty()) {
      pieces.push_back(piece);
    }

    if (found == std::string_view::npos) {
      break;
    }

    position = found + 1;
  }

  return pieces;
}

std::string join(const std::vector<std::string> &pieces, const std::string_view separator) {
  std::string joined;

  for (const auto &piece : pieces) {
    if (!joined.empty()) {
      joined.append(separator);
    }

    joined.append(piece);
  }

  return joined;
}

bool startsWith(const std::string_view value, const std::string_view prefix) {
  return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
}

bool startsWithIgnoringCase(const std::string_view value, const std::string_view prefix) {
  return value.size() >= prefix.size() && toLower(value.substr(0, prefix.size())) == toLower(prefix);
}

bool endsWithIgnoringCase(const std::string_view value, const std::string_view suffix) {
  return value.size() >= suffix.size() && toLower(value.substr(value.size() - suffix.size())) == toLower(suffix);
}

bool contains(const std::string_view haystack, const std::string_view needle) {
  return haystack.find(needle) != std::string_view::npos;
}

bool containsIgnoringCase(const std::string_view haystack, const std::string_view needle) {
  if (needle.empty()) {
    return true;
  }

  return contains(toLower(haystack), toLower(needle));
}

std::string foldAmpersand(const std::string_view value) {
  std::string folded;
  folded.reserve(value.size());

  for (std::size_t i = 0; i < value.size(); ++i) {
    if (value[i] != '&') {
      folded.push_back(value[i]);
      continue;
    }

    // TODO
    // An ampersand between two letters is part of a name rather than the word
    const auto glued =
        (i > 0 && isAlphanumeric(value[i - 1])) && (i + 1 < value.size() && isAlphanumeric(value[i + 1]));

    if (glued) {
      folded.push_back('&');
      continue;
    }

    folded.append("and");
  }

  return folded;
}

std::string stripPunctuation(const std::string_view value) {
  std::string stripped;
  stripped.reserve(value.size());

  for (const auto character : value) {
    if (isAlphanumeric(character) || isSpace(character)) {
      stripped.push_back(character);
    }
  }

  return stripped;
}

std::string restoreTrailingArticle(const std::string_view value) {
  const auto trimmed = trim(value);
  const auto comma = trimmed.rfind(", ");

  if (comma == std::string::npos) {
    return trimmed;
  }

  const auto tail = toLower(trim(trimmed.substr(comma + 2)));

  if (std::find(ARTICLES.begin(), ARTICLES.end(), tail) == ARTICLES.end()) {
    return trimmed;
  }

  return trimmed.substr(comma + 2) + " " + trimmed.substr(0, comma);
}

std::string stripLeadingArticle(const std::string_view value) {
  const auto trimmed = trim(value);

  for (const auto &article : ARTICLES) {
    if (trimmed.size() <= article.size() + 1) {
      continue;
    }

    if (toLower(trimmed.substr(0, article.size())) == article && isSpace(trimmed[article.size()])) {
      return trim(trimmed.substr(article.size()));
    }
  }

  return trimmed;
}

std::string foldRomanNumerals(const std::string_view value) {
  const auto pieces = split(value, ' ');
  std::vector<std::string> folded;
  folded.reserve(pieces.size());

  for (const auto &piece : pieces) {
    const auto lowered = toLower(piece);
    auto replacement = std::string_view{};

    for (const auto &[numeral, digits] : ROMAN_NUMERALS) {
      if (lowered == numeral) {
        replacement = digits;
        break;
      }
    }

    folded.emplace_back(replacement.empty() ? piece : std::string(replacement));
  }

  return join(folded, " ");
}

} // namespace firelight::strings

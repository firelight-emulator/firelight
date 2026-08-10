#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace firelight::strings {

/**
 * @return The string with every ASCII letter lowercased
 */
[[nodiscard]] std::string toLower(std::string_view value);

/**
 * @return The string without leading or trailing whitespace
 */
[[nodiscard]] std::string trim(std::string_view value);

/**
 * @return The string with each run of whitespace turned into one space, and the ends trimmed
 */
[[nodiscard]] std::string collapseWhitespace(std::string_view value);

/**
 * @return The string with every occurrence of from replaced by to. An empty from is returned
 *   unchanged rather than looping
 */
[[nodiscard]] std::string replaceAll(std::string_view value, std::string_view from, std::string_view to);

/**
 * @param keepEmpty Whether a run of separators produces empty pieces rather than being skipped
 * @return The pieces between separators, each trimmed
 */
[[nodiscard]] std::vector<std::string> split(std::string_view value, char separator, bool keepEmpty = false);

/**
 * @return The pieces joined by separator
 */
[[nodiscard]] std::string join(const std::vector<std::string> &pieces, std::string_view separator);

/**
 * @return Whether value begins with prefix
 */
[[nodiscard]] bool startsWith(std::string_view value, std::string_view prefix);

/**
 * @return Whether value begins with prefix, ignoring ASCII case
 */
[[nodiscard]] bool startsWithIgnoringCase(std::string_view value, std::string_view prefix);

/**
 * @return Whether needle appears anywhere in haystack
 */
[[nodiscard]] bool contains(std::string_view haystack, std::string_view needle);

/**
 * @return Whether needle appears anywhere in haystack, ignoring ASCII case. An empty needle
 *   matches anything
 */
[[nodiscard]] bool containsIgnoringCase(std::string_view haystack, std::string_view needle);

/**
 * Rewrites an ampersand as the word, so "Ratchet & Clank" and "Ratchet and Clank" agree.
 *
 * Only a standalone ampersand counts, so "AT&T" is left alone
 */
[[nodiscard]] std::string foldAmpersand(std::string_view value);

/**
 * @return The string without any character that is neither a letter, a digit, nor a space
 */
[[nodiscard]] std::string stripPunctuation(std::string_view value);

/**
 * Moves a trailing article to the front, so "Legend of Zelda, The" reads as "The Legend of
 * Zelda". Titles are written both ways depending on who dumped them
 */
[[nodiscard]] std::string restoreTrailingArticle(std::string_view value);

/**
 * @return The string without a leading "the", "a" or "an"
 */
[[nodiscard]] std::string stripLeadingArticle(std::string_view value);

/**
 * Rewrites roman numerals up to 20 as digits, so "Final Fantasy IV" and "Final Fantasy 4"
 * agree. Only a whole word counts, so "Civilization" keeps its i's.
 *
 * A sequel number is what tells two games apart, so this belongs in a key used for matching
 * rather than one used for deciding that two entries are the same game
 */
[[nodiscard]] std::string foldRomanNumerals(std::string_view value);

} // namespace firelight::strings

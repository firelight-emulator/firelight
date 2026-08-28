#pragma once

#include <set>
#include <string>
#include <string_view>

namespace firelight::library {

/**
 * The metadata fields a user edited by hand
 *
 * A scrape skips every field named here. Only field names are stored; the values stay in the document
 */
struct MetadataOverrides {
  std::set<std::string> fields;

  [[nodiscard]] bool isUserSet(std::string_view field) const;

  void markUserSet(std::string_view field);

  void clearUserSet(std::string_view field);

  /**
   * Reads an override set. Malformed input yields an empty set rather than throwing
   */
  [[nodiscard]] static MetadataOverrides parse(const std::string &json);

  /**
   * @return The field names as a JSON array
   */
  [[nodiscard]] std::string toJson() const;
};

} // namespace firelight::library

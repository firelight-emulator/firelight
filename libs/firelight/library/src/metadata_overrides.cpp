#include <firelight/library/metadata_overrides.hpp>

#include <nlohmann/json.hpp>

namespace firelight::library {

bool MetadataOverrides::isUserSet(const std::string_view field) const {
  return fields.find(std::string(field)) != fields.end();
}

void MetadataOverrides::markUserSet(const std::string_view field) { fields.insert(std::string(field)); }

void MetadataOverrides::clearUserSet(const std::string_view field) { fields.erase(std::string(field)); }

MetadataOverrides MetadataOverrides::parse(const std::string &json) {
  MetadataOverrides overrides;

  if (json.empty()) {
    return overrides;
  }

  const auto parsed = nlohmann::json::parse(json, nullptr, false);

  if (parsed.is_discarded() || !parsed.is_array()) {
    return overrides;
  }

  for (const auto &value : parsed) {
    if (value.is_string()) {
      overrides.fields.insert(value.get<std::string>());
    }
  }

  return overrides;
}

std::string MetadataOverrides::toJson() const { return nlohmann::json(fields).dump(); }

} // namespace firelight::library

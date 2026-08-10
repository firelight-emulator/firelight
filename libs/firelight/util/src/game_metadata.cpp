#include <firelight/util/game_metadata.hpp>

#include <nlohmann/json.hpp>

namespace firelight {

namespace {
void readString(const nlohmann::json &parsed, const char *key, std::string &out) {
  if (parsed.contains(key) && parsed[key].is_string()) {
    out = parsed[key].get<std::string>();
  }
}

void readStringVector(const nlohmann::json &parsed, const char *key, std::vector<std::string> &out) {
  if (!parsed.contains(key) || !parsed[key].is_array()) {
    return;
  }

  for (const auto &value : parsed[key]) {
    if (value.is_string()) {
      out.push_back(value.get<std::string>());
    }
  }
}

void writeString(nlohmann::json &out, const char *key, const std::string &value) {
  if (!value.empty()) {
    out[key] = value;
  }
}

void writeStringVector(nlohmann::json &out, const char *key, const std::vector<std::string> &value) {
  if (!value.empty()) {
    out[key] = value;
  }
}
} // namespace

GameMetadata GameMetadata::parse(const std::string &json) {
  GameMetadata metadata;

  if (json.empty()) {
    return metadata;
  }

  const auto parsed = nlohmann::json::parse(json, nullptr, false);

  if (parsed.is_discarded() || !parsed.is_object()) {
    return metadata;
  }

  readString(parsed, metadata_fields::DESCRIPTION, metadata.description);
  readString(parsed, metadata_fields::DEVELOPER, metadata.developer);
  readString(parsed, metadata_fields::PUBLISHER, metadata.publisher);
  readString(parsed, metadata_fields::RELEASE_DATE, metadata.releaseDate);
  readString(parsed, metadata_fields::PLAYERS, metadata.players);
  readString(parsed, metadata_fields::REVISION, metadata.revision);

  if (parsed.contains(metadata_fields::RELEASE_YEAR) && parsed[metadata_fields::RELEASE_YEAR].is_number_unsigned()) {
    metadata.releaseYear = parsed[metadata_fields::RELEASE_YEAR].get<unsigned>();
  }

  readStringVector(parsed, metadata_fields::GENRES, metadata.genres);
  readStringVector(parsed, metadata_fields::REGIONS, metadata.regions);
  readStringVector(parsed, metadata_fields::LANGUAGES, metadata.languages);
  readStringVector(parsed, metadata_fields::FLAGS, metadata.flags);

  return metadata;
}

std::string GameMetadata::toJson() const {
  nlohmann::json out = nlohmann::json::object();

  writeString(out, metadata_fields::DESCRIPTION, description);
  writeString(out, metadata_fields::DEVELOPER, developer);
  writeString(out, metadata_fields::PUBLISHER, publisher);
  writeString(out, metadata_fields::RELEASE_DATE, releaseDate);
  writeString(out, metadata_fields::PLAYERS, players);
  writeString(out, metadata_fields::REVISION, revision);

  if (releaseYear != 0) {
    out[metadata_fields::RELEASE_YEAR] = releaseYear;
  }

  writeStringVector(out, metadata_fields::GENRES, genres);
  writeStringVector(out, metadata_fields::REGIONS, regions);
  writeStringVector(out, metadata_fields::LANGUAGES, languages);
  writeStringVector(out, metadata_fields::FLAGS, flags);

  return out.dump();
}

bool GameMetadata::isEmpty() const {
  return description.empty() && developer.empty() && publisher.empty() && releaseYear == 0 && releaseDate.empty() &&
         players.empty() && revision.empty() && genres.empty() && regions.empty() && languages.empty() && flags.empty();
}

} // namespace firelight
